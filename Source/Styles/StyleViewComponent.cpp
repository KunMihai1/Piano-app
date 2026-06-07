#include "displayGUI.h"

StyleViewComponent::StyleViewComponent(const juce::String& styleName)
{
    label.setFont(juce::Font(18.0f, juce::Font::bold));
    label.setText(styleName, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, AppColours::accent3);
    label.setInterceptsMouseClicks(true, false);
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addAndMakeVisible(label);

    label.addMouseListener(this, false);

}

void StyleViewComponent::resized()
{
    auto font = label.getFont();
    int textWidth = juce::GlyphArrangement::getStringWidthInt(font, label.getText());
    int textHeight = static_cast<int>(font.getHeight());

    label.setBounds(10, 10, textWidth + 10, textHeight + 6);
}

void StyleViewComponent::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &label && onStyleClicked)
    {
        if (event.mods.isLeftButtonDown())
        {
            if(onStyleClicked)
                onStyleClicked(label.getText());
        }
        else if (event.mods.isRightButtonDown())
        {
            juce::PopupMenu menu;
            if (label.getText() == "DEFAULT Style")
            {
                menu.addItem("Can't change", [this]() {

                    });
            }
            else
            {
                menu.addItem("Rename", [this]() {
                    changeNameLabel();
                    });

                menu.addItem("Delete", [this]() {
                    removeStyle();
                    });


            }
            menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this));
        }
    }
}

void StyleViewComponent::setNameLabel(const juce::String& name)
{
    label.setText(name, juce::dontSendNotification);
}

juce::String StyleViewComponent::getNameLabel() const
{
    return label.getText();
}

void StyleViewComponent::changeNameLabel()
{
    auto* window = new juce::AlertWindow{ "Rename track","Enter a name for your track",juce::AlertWindow::NoIcon };

    window->addTextEditor("nameEditor", label.getText(), "Style Name:");

    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    window->enterModalState(true, juce::ModalCallbackFunction::create([this, window](int result)
        {
            std::unique_ptr<juce::AlertWindow> cleanup{ window };
            if (result != 1)
                return;

            juce::String oldName = label.getText();
            juce::String theNewName = window->getTextEditor("nameEditor")->getText().trim();

            if (theNewName.isEmpty())
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Rename style", "You can't put an empty name!");
                return;
            }

            if (isInListNames)
            {
                if (isInListNames(theNewName))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Rename style", "The name already exists!");
                    return;
                }
            }

            setNameLabel(theNewName);

            if (onStyleRenamed)
                onStyleRenamed(oldName, theNewName);


        }
    ));

    juce::MessageManager::callAsync([window]()
        {
            if (auto* editor = window->getTextEditor("nameEditor"))
                editor->grabKeyboardFocus();
        });
}

void StyleViewComponent::removeStyle()
{
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::WarningIcon,
        "Confirm Remove",
        "Are you sure you want to remove ' "+label.getText()+ "' ?",
        "Remove",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this](int result)
            {
                if (result == 0) // Cancel
                    return;

                if (onStyleRemoveComponent)
                    onStyleRemoveComponent(label.getText());

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Remove style", "Style removed succesfully!");

            }
        )
    );
}
