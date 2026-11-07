#pragma once
#include <JuceHeader.h>
#include "AboutContent.h"
#include "Theme.h"

// Scrollable "page" that draws headings / paragraphs / bullet lists and clickable link tables.
class AboutPage : public juce::Component
{
public:
    explicit AboutPage (std::vector<about::Item> content) : items (std::move (content))
    {
        for (auto& it : items)
            for (auto& r : it.rows)
            {
                auto b = std::make_unique<juce::HyperlinkButton> (r.text, juce::URL (r.url));
                b->setFont (juce::Font (15.0f), false, juce::Justification::centredLeft);
                b->setColour (juce::HyperlinkButton::textColourId, theme::blueH);
                b->setTooltip (r.url);
                addAndMakeVisible (*b);
                links.push_back (std::move (b));
            }
    }

    int doLayout (int width)
    {
        cells.clear();
        texts.clear();
        const int margin = 22, tw = width - 2 * margin;
        int y = 18;
        size_t linkIdx = 0;
        for (auto& it : items)
        {
            switch (it.type)
            {
                case about::Item::H2:    y = addText (it, juce::Font (24.0f, juce::Font::bold), theme::text, margin, y, tw, 4, 8); break;
                case about::Item::H3:    y = addText (it, juce::Font (17.0f, juce::Font::bold), theme::blueH, margin, y + 8, tw, 2, 4); break;
                case about::Item::Bold:  y = addText (it, juce::Font (15.0f, juce::Font::bold), theme::text, margin, y, tw, 2, 3); break;
                case about::Item::Muted: y = addText (it, juce::Font (14.0f), theme::muted, margin, y, tw, 2, 6); break;
                case about::Item::Para:  y = addText (it, juce::Font (15.0f), theme::soft, margin, y, tw, 2, 8); break;
                case about::Item::Bullet:
                {
                    texts.push_back ({ {}, juce::String::fromUTF8 ("\xE2\x80\xA2"), juce::Font (15.0f), theme::blueH, { margin + 4, y, 14, 22 } });
                    y = addText (it, juce::Font (15.0f), theme::soft, margin + 22, y, tw - 22, 2, 4);
                    break;
                }
                case about::Item::Links:
                {
                    y += 4;
                    for (auto& r : it.rows)
                    {
                        const int h = 38, lw = (int) (tw * 0.30f);
                        cells.push_back ({ { margin, y, lw, h }, theme::panel3, r.label, true });
                        cells.push_back ({ { margin + lw + 4, y, tw - lw - 4, h }, theme::panel2, {}, false });
                        links[linkIdx++]->setBounds (margin + lw + 14, y, tw - lw - 24, h);
                        y += h + 4;
                    }
                    y += 6;
                    break;
                }
            }
        }
        return y + 20;
    }

    void resized() override { doLayout (getWidth()); repaint(); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (theme::panel);
        for (auto& c : cells)
        {
            g.setColour (c.colour);
            g.fillRoundedRectangle (c.area.toFloat(), 5.0f);
            if (c.label.isNotEmpty())
            {
                g.setColour (theme::text);
                g.setFont (juce::Font (15.0f, juce::Font::bold));
                g.drawText (c.label, c.area.reduced (10, 0), juce::Justification::centredLeft);
            }
        }
        for (auto& t : texts)
        {
            if (t.layout.getNumLines() > 0)
                t.layout.draw (g, t.area.toFloat());
            else
            {
                g.setColour (t.colour);
                g.setFont (t.font);
                g.drawText (t.plain, t.area, juce::Justification::topLeft);
            }
        }
    }

private:
    struct Cell { juce::Rectangle<int> area; juce::Colour colour; juce::String label; bool isLabel; };
    struct Text { juce::TextLayout layout; juce::String plain; juce::Font font; juce::Colour colour; juce::Rectangle<int> area; };

    int addText (const about::Item& it, const juce::Font& f, juce::Colour c, int x, int y, int w, int padTop, int padBottom)
    {
        juce::AttributedString as;
        as.setWordWrap (juce::AttributedString::byWord);
        as.append (it.text, f, c);
        juce::TextLayout tl;
        tl.createLayout (as, (float) w);
        const int h = (int) std::ceil (tl.getHeight()) + 2;
        texts.push_back ({ std::move (tl), {}, f, c, { x, y + padTop, w, h } });
        return y + padTop + h + padBottom;
    }

    std::vector<about::Item> items;
    std::vector<std::unique_ptr<juce::HyperlinkButton>> links;
    std::vector<Cell> cells;
    std::vector<Text> texts;
};

class AboutContentComponent : public juce::Component
{
public:
    explicit AboutContentComponent (std::vector<about::Item> content)
        : page (std::move (content))
    {
        viewport.setViewedComponent (&page, false);
        viewport.setScrollBarsShown (true, false);
        viewport.setScrollBarThickness (11);
        addAndMakeVisible (viewport);

        closeButton.setButtonText ("Close");
        closeButton.onClick = [this]
        {
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState (0);
        };
        addAndMakeVisible (closeButton);
        setSize (700, 640);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto bottom = r.removeFromBottom (54).reduced (14, 10);
        closeButton.setBounds (bottom.removeFromRight (110));
        viewport.setBounds (r);
        const int w = viewport.getMaximumVisibleWidth();
        page.setSize (w, page.doLayout (w));
    }

    void paint (juce::Graphics& g) override { g.fillAll (theme::bg); }

private:
    AboutPage page;
    juce::Viewport viewport;
    juce::TextButton closeButton;
};

inline void showAboutWindow (const juce::String& title, std::vector<about::Item> content, juce::Component* around)
{
    juce::DialogWindow::LaunchOptions o;
    o.dialogTitle = title;
    o.dialogBackgroundColour = theme::bg;
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = true;
    o.componentToCentreAround = around;
    o.content.setOwned (new AboutContentComponent (std::move (content)));
    o.launchAsync();
}
