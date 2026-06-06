#include "displayGUI.h"

StylesListComponent::StylesListComponent(std::vector<juce::String> stylesNamesOut, std::function<void(const juce::String&)> onStyleClicked, int widthSize) : stylesNames{stylesNamesOut}, onStyleClicked{onStyleClicked}
{
    addAndMakeVisible(addButton);

    addButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addButton.setLookAndFeel(&laf);
    addButton.setColour(juce::TextButton::buttonColourId, AppColours::accent7);

    addButton.onClick = [this]()
    {
        addNewStyle();
    };

    this->widthSize = widthSize;

    styleItemsContainer = std::make_unique<juce::Component>();
    viewport.setViewedComponent(styleItemsContainer.get(), false);
    addAndMakeVisible(viewport);

    populate();
}

void StylesListComponent::resized()
{

    this->widthSize = getWidth();

    const int controlBarHeight = 24;
    const int buttonWidth = 80;
    const int buttonHeight = 20;
    const int spacing = 10;

    addButton.setBounds(spacing/2, (controlBarHeight - buttonHeight) / 2, buttonWidth, buttonHeight);
    viewport.setBounds(0, controlBarHeight, getWidth(), getHeight() - controlBarHeight);

    layoutStyles();
}

void StylesListComponent::paint(juce::Graphics& g)
{
    g.fillAll(AppColours::panelBg);

    const int controlBarHeight = 24;

    juce::ColourGradient gradient(AppColours::cardBg.brighter(0.05f), 0, 0,
                                  AppColours::cardBg, 0, (float)controlBarHeight, false);
    g.setGradientFill(gradient);
    g.fillRect(0, 0, getWidth(), controlBarHeight);

    g.setColour(AppColours::separator);
    g.drawLine(0.0f, (float)(controlBarHeight - 1), (float)getWidth(), (float)(controlBarHeight - 1), 1.0f);
}

void StylesListComponent::setWidthSize(const int newWidth)
{
    this->widthSize = newWidth;
}

void StylesListComponent::layoutStyles()
{
    const int columns = 2;
    const int spacing = 10;
    const int sidePadding = spacing;

    const int totalSpacing = (columns - 1) * spacing + 2 * sidePadding;
    const int itemWidth = (getWidth() - totalSpacing) / columns;
    const int itemHeight = 50;

    int x = sidePadding;
    int y = spacing;

    int count = 0;
    for (auto* style : allStyles)
    {
        style->setBounds(x, y, itemWidth, itemHeight);
        count++;

        if (count % columns == 0)
        {
            x = sidePadding;
            y += itemHeight + spacing;
        }
        else
        {
            x += itemWidth + spacing;
        }
    }

    int rows = static_cast<int>(std::ceil(allStyles.size() / (float)columns));
    int totalHeight = rows * itemHeight + (rows + 1) * spacing;

    styleItemsContainer->setSize(getWidth(), totalHeight);
}

void StylesListComponent::repopulate()
{
    stylesNames.clear();
    for (auto* sv : allStyles)
        stylesNames.push_back(sv->getNameLabel());

    populate();
    resized();
}

void StylesListComponent::addNewStyle()
{
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon };

    window->addTextEditor("nameEditor", "", "Style Name:");;

    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
            {
                std::unique_ptr<juce::AlertWindow> cleanup{ window };
                if (result != 1)
                    return;

                juce::String newName = window->getTextEditor("nameEditor")->getText().trim();

                if (newName.isEmpty())
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Adding a style", "Name can't be empty!");
                    return;
                }

                for (auto& name : stylesNames)
                {
                    if (name.trim() == newName)
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Adding a style", "Name already exists!");
                        return;
                    }
                }

                auto* newStyle = new StyleViewComponent{ newName };

                juce::String currentName = newName;

                allCallBacks(newStyle,currentName);

                if (onStyleAdd)
                    onStyleAdd(newName);

                stylesNames.push_back(newName);
                styleItemsContainer->addAndMakeVisible(newStyle);
                allStyles.add(newStyle);
                resized(); // instead of repopulating, we just layout the styles again ( repopulating would mean creating all the lambdas again and so on )

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Add style", "The style '" + newName + "' has been added!");
            }
        )
    );

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
}

void StylesListComponent::allCallBacks(StyleViewComponent* newStyle, const juce::String& currentName)
{
    newStyle->onStyleClicked = this->onStyleClicked;

    newStyle->onStyleRenamed = [this](const juce::String& oldName, const juce::String& newName)
    {
        onStyleRename(oldName, newName);
    };

    newStyle->onStyleRemoveComponent = [this](const juce::String& name)
    {
        removeStyleLocally(name);
    };

    newStyle->isInListNames = [this, currentName](const juce::String& newName)
    {
        bool isIn = false;
        for (auto& nameInList : stylesNames)
        {
            if (nameInList == newName && newName != currentName)
            {
                isIn = true;
                break;
            }
        }
        return isIn;
    };
}

void StylesListComponent::removeStyleLocally(const juce::String& name)
{
    for (int i = 0; i < allStyles.size(); ++i)
    {
        if (allStyles[i]->getNameLabel() == name)
        {
            auto* comp = allStyles[i];
            styleItemsContainer->removeChildComponent(comp);
            allStyles.remove(i);
            break;
        }
    }

    stylesNames.clear();
    for (auto* sv : allStyles)
        stylesNames.push_back(sv->getNameLabel());

    if (onStyleRemove)
        onStyleRemove(name);

    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    populate();
    resized();
}

void StylesListComponent::addStyleLocally(const juce::String& newName)
{
    stylesNames.push_back(newName);

    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    populate();
    resized();
}

void StylesListComponent::rebuildStyleNames()
{
    stylesNames.clear();
    for (auto& style : allStyles)
    {
        juce::String name = style->getNameLabel();
        stylesNames.push_back(name);
    }
}


void StylesListComponent::populate()
{
    styleItemsContainer->removeAllChildren();
    allStyles.clear();
    for (int i = 0; i < stylesNames.size(); i++)
    {
        juce::String name;
        if (i < stylesNames.size())
            name = stylesNames[i];
        else name = "Style " + juce::String(i);
        auto* newStyle = new StyleViewComponent{ name };

        juce::String currentName = name;

        allCallBacks(newStyle, currentName);

        styleItemsContainer->addAndMakeVisible(newStyle);
        allStyles.add(newStyle);
    }
}
