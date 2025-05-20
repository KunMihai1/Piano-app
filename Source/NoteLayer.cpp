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
    keyBoardUI.midiHandler.addListener(this);

    openGLContext.setRenderer(this);
    openGLContext.attachTo(*this);
}

NoteLayer::~NoteLayer()
{
    keyBoardUI.midiHandler.removeListener(this);
    openGLContext.detach();
}

void NoteLayer::paint(juce::Graphics& g)
{
    for (const auto& [midiNote, note] : activeNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);
    }
    for (const auto& note : fallingNotes)
    {
        g.setColour(juce::Colours::green.withAlpha(note.alpha));
        g.fillRoundedRectangle(note.bounds.toFloat(), 6.0f);
    }
}

void NoteLayer::noteOnReceived(int midiNote)
{
    juce::MessageManager::callAsync([this, midiNote]
        {
            if (activeNotes.find(midiNote) == activeNotes.end())
            {
                AnimatedNote newNote;
                newNote.bounds = keyBoardUI.keys[midiNote].bounds;

                newNote.height = 10.0f;
                newNote.yPosition = static_cast<float>(getHeight()) - newNote.height;

                newNote.bounds.setHeight(static_cast<int>(round(newNote.height)));
                newNote.bounds.setY(static_cast<int>(round(newNote.yPosition)));

                activeNotes[midiNote] = newNote;

                spawnParticlesForNote(midiNote);

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
        void main()
        {
            float dist = length(gl_PointCoord - vec2(0.5));
            float alpha = smoothstep(0.5, 0.0, dist);
            gl_FragColor = vColour * alpha;
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
        glViewport(0, 0, getWidth(), getHeight());
    }
    else {
        openGLContext.makeActive();
        glEnable(GL_PROGRAM_POINT_SIZE);
        glViewport(0, 0, getWidth(), getHeight());
    }
    openGLContext.extensions.glGenBuffers(1, &particleVBO);
}

void NoteLayer::renderOpenGL()
{
    DBG("renderOpenGL() called");

    if (shader != nullptr && particleVBO != 0 && !particles.empty())
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
            verts.push_back({ p.pos.x, p.pos.y, p.size,
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
    float y_ndc = 1.0f - (keyBounds.getY() / (float)getHeight()) * 2.0f;

    juce::Point<float> pos(x_ndc, y_ndc);

    for (int i = 0; i < 10; i++)
    {
        Particle p;
        p.pos = pos;

        // Velocity should be small since NDC is from -1 to 1
        p.velocity = {
            (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * 0.5f,  // horizontal velocity in NDC units/sec
            -(juce::Random::getSystemRandom().nextFloat() * 1.0f)                // vertical velocity in NDC units/sec
        };

        p.size = 10.0f + juce::Random::getSystemRandom().nextFloat() * 5.0f; // size in pixels
        p.colour = juce::Colour::fromHSV(juce::Random::getSystemRandom().nextFloat(), 1.0f, 1.0f, 1.0f);
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
    const float riseSpeed = 350.0f;
    const float fadeSpeed = 350.0f;
    const float fadeTime = 5.0f;
    const float fadeRate = 1.0f / fadeTime;

    for (auto& pair : activeNotes)
    {
        AnimatedNote& n = pair.second;

        n.yPosition -= riseSpeed * dt;
        n.height += riseSpeed * dt;

        n.bounds.setY(static_cast<int>(round(n.yPosition)));
        n.bounds.setHeight(static_cast<int>(round(n.height)));
    }

    for (auto it = fallingNotes.begin(); it != fallingNotes.end();)
    {
        AnimatedNote& n = *it;

        n.alpha -= dt * fadeRate;

        n.yPosition -= fadeSpeed * dt;
        n.bounds.setY(static_cast<int>(round(n.yPosition)));

        if (it->alpha <= 0.0f)
            it = fallingNotes.erase(it);
        else ++it;
    }
    updateParticles();

    /*
    particles.clear();

    // Place one test particle in center of screen (NDC = [0,0])
    Particle p;
    p.pos = { 0.0f, 0.0f };     // center in NDC
    p.velocity = { 0.0f, 0.0f };     // no motion
    p.size = 40.0f;             // big so you can see it
    p.colour = juce::Colours::red;
    p.life = 1.0f;
    particles.push_back(p);

    */
    if (!particles.empty() || !activeNotes.empty() || !fallingNotes.empty())
    {
        openGLContext.triggerRepaint();
        repaint();
    }
    else {
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
