/*
  ==============================================================================

    NoteLayer.cpp
    Created: 20 May 2025 1:36:54pm
    Author:  Kisuke

  ==============================================================================
*/


#include "NoteLayer.h" 
#include <juce_opengl/juce_opengl.h>

using namespace juce::gl;

NoteLayer::NoteLayer(KeyboardUI& referenceKeyboard) : keyBoardUI{ referenceKeyboard }
{
    setSize(getWidth(), getHeight());
    //keyBoardUI.midiHandler.addListener(this);

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);

    isActive = true;
    float hue = 0.12f;            // yellow-gold hue
    float saturation = 0.85f + juce::Random::getSystemRandom().nextFloat() * 0.1f;  // 0.85 - 0.95
    float brightness = 0.9f + juce::Random::getSystemRandom().nextFloat() * 0.1f;   // 0.9 - 1.0
    float alpha = 0.6f + juce::Random::getSystemRandom().nextFloat() * 0.4f;        // 0.6 - 1.0
    this->particleColourUser = juce::Colour::fromHSV(hue, saturation, brightness, alpha);
}

NoteLayer::~NoteLayer()
{
    //keyBoardUI.midiHandler.removeListener(this);
    openGLContext.detach();
}

void NoteLayer::paint(juce::Graphics& g)
{
    g.reduceClipRegion(getLocalBounds());
    for (const auto& [midiNote, note] : activeNotes)
    {
        g.setColour(juce::Colours::transparentBlack.withAlpha(0.1f));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);

        g.setColour(juce::Colours::whitesmoke.withAlpha(1.0f));
        g.drawRoundedRectangle(note.bounds.toFloat(), 6.0f, 1.5f); // 1.5f = outline thickness
    }
    for (const auto& note : fallingNotes)
    {
        g.setColour(juce::Colours::transparentBlack.withAlpha(note.alpha));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);

        // Outline for falling notes
        g.setColour(juce::Colours::whitesmoke.withAlpha(note.alpha));
        g.drawRoundedRectangle(note.bounds.toFloat(), 6.0f, 1.5f);;
    }
}

void NoteLayer::noteOnReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            if (activeNotes.find(midiNote) == activeNotes.end())
            {
                if (!this->isVisible() && keyBoardUI.isVisible())
                    this->setVisible(true);
                AnimatedNote newNote;
                newNote.bounds = keyBoardUI.keys[midiNote].bounds;

                newNote.height = 10.0f;
                newNote.yPosition = static_cast<float>(getHeight()) - newNote.height;

                newNote.bounds.setHeight(static_cast<int>(round(newNote.height)));
                newNote.bounds.setY(static_cast<int>(round(newNote.yPosition)));

                activeNotes[midiNote] = newNote;

                if(!isTimerRunning())
                    startTimerHz(60);
            }
        });
}

void NoteLayer::noteOffReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            auto it = activeNotes.find(midiNote);
            if (it != activeNotes.end())
            {
                it->second.isFalling = true;
                it->second.alpha = 1.0f;
                it->second.initialHeight = it->second.height;

                //DBG("Initial height"<<it->second.initialHeight);
                fallingNotes.push_back(it->second);
                activeNotes.erase(it);
            }
        });
}

void NoteLayer::newOpenGLContextCreated()
{
    const char* vertexSource = R"VERT(
        attribute vec2 position;
    attribute float pointSize;
    attribute vec4 colour;
    varying vec4 vColour;

    void main()
    {
        gl_PointSize = pointSize;
        gl_Position = vec4(position, 0.0, 1.0);
        vColour = colour;
    }
    )VERT";

    const char* fragmentSource = R"FRAG(
#ifdef GL_ES
precision mediump float;
#endif

varying vec4 vColour;
varying vec2 vTexCoord;
uniform float uTime;

void main()
{
    // Normalize texture coordinates to range [0.0, 1.0]
    vec2 uv = vTexCoord;

    // Calculate distance from the center
    float dist = length(uv - vec2(0.5));

    // Apply smoothstep for soft radial falloff
    float alpha = smoothstep(0.4, 0.6, dist);
    //float alpha = smoothstep(0.5, 0.0, dist);

    // Introduce a time-based color shift using sine wave modulation
    vec3 color = vColour.rgb * (0.5 + 0.5 * sin(uTime + dist * 10.0));

    // Output the final color with alpha transparency
    gl_FragColor = vec4(color, alpha);
}
)FRAG";

    shader = std::make_unique<juce::OpenGLShaderProgram>(openGLContext);
    if (!shader->addVertexShader(vertexSource) || !shader->addFragmentShader(fragmentSource) || !shader->link())
    {
        jassertfalse;
        shader.reset();
        return;
    }
    positionAttr = new juce::OpenGLShaderProgram::Attribute(*shader, "position");
    pointSizeAttr = new juce::OpenGLShaderProgram::Attribute(*shader, "pointSize");
    colourAttr = new juce::OpenGLShaderProgram::Attribute(*shader, "colour");

    if (openGLContext.isActive())
    {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, getWidth(), getHeight());
    }
    else {
        openGLContext.makeActive();
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0, 0, getWidth(), getHeight());
    }
    openGLContext.extensions.glGenBuffers(1, &particleVBO);
}

void NoteLayer::renderOpenGL()
{
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (shader != nullptr && particleVBO != 0 && !particles.empty())
    {
        shader->use();

        struct Vertex {
            float x, y;
            float size;
            float r, g, b, a;
        };

        std::vector<Vertex> verts;
        verts.reserve(particles.size());

        for (const auto& p : particles)
        {
            verts.push_back({ p.pos.x, p.pos.y, p.size*0.5f,
                              p.colour.getFloatRed(),
                              p.colour.getFloatGreen(),
                              p.colour.getFloatBlue(),
                              p.colour.getFloatAlpha() * p.life });
        }

        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)* verts.size(), verts.data(), GL_DYNAMIC_DRAW);

        if (positionAttr != nullptr)
        {

            openGLContext.extensions.glVertexAttribPointer(positionAttr->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, x));
            openGLContext.extensions.glEnableVertexAttribArray(positionAttr->attributeID);
        }
        if (pointSizeAttr != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer(pointSizeAttr->attributeID, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, size));
            openGLContext.extensions.glEnableVertexAttribArray(pointSizeAttr->attributeID);
        }
        if (colourAttr != nullptr)
        {
            openGLContext.extensions.glVertexAttribPointer(colourAttr->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, r));
            openGLContext.extensions.glEnableVertexAttribArray(colourAttr->attributeID);
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive blend

        glDrawArrays(GL_POINTS, 0, (GLsizei)verts.size());

        // Cleanup
        if (positionAttr) openGLContext.extensions.glDisableVertexAttribArray(positionAttr->attributeID);
        if (pointSizeAttr) openGLContext.extensions.glDisableVertexAttribArray(pointSizeAttr->attributeID);
        if (colourAttr) openGLContext.extensions.glDisableVertexAttribArray(colourAttr->attributeID);
    }
}

void NoteLayer::openGLContextClosing()
{
    if (particleVBO)
    {
        openGLContext.extensions.glDeleteBuffers(1, &particleVBO);
        particleVBO = 0;
    }
    delete positionAttr;
    positionAttr = nullptr;

    delete pointSizeAttr;
    pointSizeAttr = nullptr;

    delete colourAttr;
    colourAttr = nullptr;

    shader.reset();
}

void NoteLayer::resetState()
{
    this->activeNotes.clear();
    this->fallingNotes.clear();
    this->particles.clear();
}

void NoteLayer::resetStateActiveNotes()
{
    for (auto& v : activeNotes)
    {
        int note = v.first;
        noteOffReceived(note);
    }
}

void NoteLayer::setColourParticle(juce::Colour& colour)
{
    this->particleColourUser = colour;
}

void NoteLayer::updateParticles()
{
    float dt = 1.0f / 60.0f;
    for (auto& p : particles)
    {
        p.pos += p.velocity * dt;
        p.life -= dt * 0.25f;
    }


    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p)
        {
            return p.life <= 0.0f;
        }),
        particles.end());
}

void NoteLayer::spawnParticlesForNote(int midiNote)
{
    auto keyBounds = this->keyBoardUI.keys[midiNote].bounds;

    // Convert pixel center to NDC [-1, 1]
    float x_ndc = (keyBounds.getCentreX() / (float)getWidth()) * 2.0f - 1.0f;
    float y_ndc = -(1.0f - (keyBounds.getY() / (float)getHeight()) * 2.0f);
    y_ndc += 0.05f;
    juce::Point<float> pos(x_ndc, y_ndc);

    for (int i = 0; i < 1; i++)
    {
        Particle p;
        p.pos = pos;

        // Velocity should be small since NDC is from -1 to 1
        /*
        p.velocity = {
            (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * 0.5f,  // horizontal velocity in NDC units/sec
            -(juce::Random::getSystemRandom().nextFloat() * 1.0f)                // vertical velocity in NDC units/sec
        };
        */
        p.velocity = {
        (juce::Random::getSystemRandom().nextFloat() - 0.5f) * 0.1f, // subtle wiggle
        juce::Random::getSystemRandom().nextFloat() * 0.8f - 0.3f   // upward, fast
        };

        p.size = 1.0f + juce::Random::getSystemRandom().nextFloat() * 8.0f;

        p.colour = this->particleColourUser;
        p.life = 1.0f;
        particles.push_back(p);
    }
}


void NoteLayer::visibilityChanged()
{
    if (!isVisible())
        stopTimer();
    //else startTimerHz(60);
}

void NoteLayer::timerCallback()
{
    const float dt = 1.0f / 60.0f;
    const float riseSpeed = 400.0f;
    const float fadeSpeedBase = 400.0f;
    const float fadeTimeBase = 6.0f;
    const float shrinkSpeedBase = 150.0f;
    const float windowHeight = static_cast<float>(getHeight());

    const float maxNoteHeight = getHeight(); // or getHeight() if dynamic

    for (auto& pair : activeNotes)
    {
        AnimatedNote& n = pair.second;

        n.yPosition -= riseSpeed * dt;
        n.height += riseSpeed * dt;

        // Clamp growth
        if (n.height > maxNoteHeight)
        {
            float bottom = n.yPosition + n.height;
            n.height = maxNoteHeight;
            n.yPosition = bottom - n.height;
        }

        n.bounds.setY(static_cast<int>(std::round(n.yPosition)));
        n.bounds.setHeight(static_cast<int>(std::round(n.height)));

        spawnParticlesForNote(pair.first);
    }

    // Update falling notes (released notes)
    for (auto it = fallingNotes.begin(); it != fallingNotes.end(); )
    {
        AnimatedNote& n = *it;

        // Scale fade time and fade rate based on initial height relative to window height
        float fadeTimeScaled = fadeTimeBase * (n.initialHeight / windowHeight);
        fadeTimeScaled = std::max(1.0f, fadeTimeScaled); // minimum 1 sec fade
        float fadeRateScaled = 1.0f / fadeTimeScaled;

        // Scale shrink speed similarly
        float shrinkSpeedScaled = shrinkSpeedBase * (n.initialHeight / windowHeight);

        // Fade alpha
        n.alpha = std::max(0.0f, n.alpha - dt * fadeRateScaled);

        // Move upward
        n.yPosition -= fadeSpeedBase * dt;

        // Shrink height WITHOUT clamping to 1.0f here
        float bottom = n.yPosition + n.height;
        n.height -= shrinkSpeedScaled * dt;

        // Clamp height not below tiny positive number internally
        if (n.height < 0.01f)
            n.height = 0.01f;

        // Keep bottom fixed so note shrinks upwards
        n.yPosition = bottom - n.height;

        // Clamp height for rendering (at least 1 pixel)
        int yInt = static_cast<int>(std::round(n.yPosition));
        int hInt = std::max(1, static_cast<int>(std::round(n.height)));
        n.bounds.setY(yInt);
        n.bounds.setHeight(hInt);

        // Erase note if fully faded OR fully shrunk internally
        if (n.alpha <= 0.001f || n.height <= 0.01f)
        {
            //DBG("NOTE OFF");
            it = fallingNotes.erase(it);
        }
        else
        {
            ++it;
        }
    }


    updateParticles();

    if (!particles.empty() || !activeNotes.empty() || !fallingNotes.empty())
    {
        openGLContext.triggerRepaint();
        repaint();
    }
    else
    {
        particles.clear();
        fallingNotes.clear();
        activeNotes.clear();
        openGLContext.triggerRepaint();
        repaint();

        stopTimer();
    }
}

void NoteLayer::resized()
{
    if (openGLContext.isActive())
    {
        glViewport(0, 0, getWidth(), getHeight());
    }
    else
    {
        openGLContext.makeActive();
        glViewport(0, 0, getWidth(), getHeight());
    }
}