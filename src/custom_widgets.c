/*
#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

static void GuiDrawTextEx(const char *text, Rectangle textBounds, int alignment, Color tint)
{
    #define TEXT_VALIGN_PIXEL_OFFSET(h)  ((int)h%2)     // Vertical alignment for pixel perfect

    #if !defined(ICON_TEXT_PADDING)
        #define ICON_TEXT_PADDING   4
    #endif

    if ((text == NULL) || (text[0] == '\0')) return;    // Security check

    // PROCEDURE:
    //   - Text is processed line per line
    //   - For every line, horizontal alignment is defined
    //   - For all text, vertical alignment is defined (multiline text only)
    //   - For every line, wordwrap mode is checked (useful for GuitextBox(), read-only)

    // Get text lines (using '\n' as delimiter) to be processed individually
    // WARNING: We can't use GuiTextSplit() function because it can be already used
    // before the GuiDrawText() call and its buffer is static, it would be overriden :(
    int lineCount = 0;
    const char **lines = GetTextLines(text, &lineCount);

    // Text style variables
    //int alignment = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
    int alignmentVertical = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL);
    int wrapMode = GuiGetStyle(DEFAULT, TEXT_WRAP_MODE);    // Wrap-mode only available in read-only mode, no for text editing

    // TODO: WARNING: This totalHeight is not valid for vertical alignment in case of word-wrap
    float totalHeight = (float)(lineCount*GuiGetStyle(DEFAULT, TEXT_SIZE) + (lineCount - 1)*GuiGetStyle(DEFAULT, TEXT_SIZE)/2);
    float posOffsetY = 0.0f;

    for (int i = 0; i < lineCount; i++)
    {
        int iconId = 0;
        lines[i] = GetTextIcon(lines[i], &iconId);      // Check text for icon and move cursor

        // Get text position depending on alignment and iconId
        //---------------------------------------------------------------------------------
        Vector2 textBoundsPosition = { textBounds.x, textBounds.y };
        float textBoundsWidthOffset = 0.0f;

        // NOTE: We get text size after icon has been processed
        // WARNING: GetTextWidth() also processes text icon to get width! -> Really needed?
        int textSizeX = GetTextWidth(lines[i]);

        // If text requires an icon, add size to measure
        if (iconId >= 0)
        {
            textSizeX += RAYGUI_ICON_SIZE*guiIconScale;

            // WARNING: If only icon provided, text could be pointing to EOF character: '\0'
#if !defined(RAYGUI_NO_ICONS)
            if ((lines[i] != NULL) && (lines[i][0] != '\0')) textSizeX += ICON_TEXT_PADDING;
#endif
        }

        // Check guiTextAlign global variables
        switch (alignment)
        {
            case TEXT_ALIGN_LEFT: textBoundsPosition.x = textBounds.x; break;
            case TEXT_ALIGN_CENTER: textBoundsPosition.x = textBounds.x +  textBounds.width/2 - textSizeX/2; break;
            case TEXT_ALIGN_RIGHT: textBoundsPosition.x = textBounds.x + textBounds.width - textSizeX; break;
            default: break;
        }

        if (textSizeX > textBounds.width && (lines[i] != NULL) && (lines[i][0] != '\0')) textBoundsPosition.x = textBounds.x;

        switch (alignmentVertical)
        {
            // Only valid in case of wordWrap = 0;
            case TEXT_ALIGN_TOP: textBoundsPosition.y = textBounds.y + posOffsetY; break;
            case TEXT_ALIGN_MIDDLE: textBoundsPosition.y = textBounds.y + posOffsetY + textBounds.height/2 - totalHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(textBounds.height); break;
            case TEXT_ALIGN_BOTTOM: textBoundsPosition.y = textBounds.y + posOffsetY + textBounds.height - totalHeight + TEXT_VALIGN_PIXEL_OFFSET(textBounds.height); break;
            default: break;
        }

        // NOTE: Make sure we get pixel-perfect coordinates,
        // In case of decimals we got weird text positioning
        textBoundsPosition.x = (float)((int)textBoundsPosition.x);
        textBoundsPosition.y = (float)((int)textBoundsPosition.y);
        //---------------------------------------------------------------------------------

        // Draw text (with icon if available)
        //---------------------------------------------------------------------------------
#if !defined(RAYGUI_NO_ICONS)
        if (iconId >= 0)
        {
            // NOTE: We consider icon height, probably different than text size
            GuiDrawIcon(iconId, (int)textBoundsPosition.x, (int)(textBounds.y + textBounds.height/2 - RAYGUI_ICON_SIZE*guiIconScale/2 + TEXT_VALIGN_PIXEL_OFFSET(textBounds.height)), guiIconScale, tint);
            textBoundsPosition.x += (float)(RAYGUI_ICON_SIZE*guiIconScale + ICON_TEXT_PADDING);
            textBoundsWidthOffset = (float)(RAYGUI_ICON_SIZE*guiIconScale + ICON_TEXT_PADDING);
        }
#endif
        // Get size in bytes of text,
        // considering end of line and line break
        int lineSize = 0;
        for (int c = 0; (lines[i][c] != '\0') && (lines[i][c] != '\n') && (lines[i][c] != '\r'); c++, lineSize++){ }
        float scaleFactor = (float)GuiGetStyle(DEFAULT, TEXT_SIZE)/guiFont.baseSize;

        int lastSpaceIndex = 0;
        bool tempWrapCharMode = false;

        int textOffsetY = 0;
        float textOffsetX = 0.0f;
        float glyphWidth = 0;

        int ellipsisWidth = GetTextWidth("...");
        bool textOverflow = false;
        for (int c = 0, codepointSize = 0; c < lineSize; c += codepointSize)
        {
            int codepoint = GetCodepointNext(&lines[i][c], &codepointSize);
            int index = GetGlyphIndex(guiFont, codepoint);

            // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
            // but we need to draw all of the bad bytes using the '?' symbol moving one byte
            if (codepoint == 0x3f) codepointSize = 1; // TODO: Review not recognized codepoints size

            // Get glyph width to check if it goes out of bounds
            if (guiFont.glyphs[index].advanceX == 0) glyphWidth = ((float)guiFont.recs[index].width*scaleFactor);
            else glyphWidth = (float)guiFont.glyphs[index].advanceX*scaleFactor;

            // Wrap mode text measuring, to validate if
            // it can be drawn or a new line is required
            if (wrapMode == TEXT_WRAP_CHAR)
            {
                // Jump to next line if current character reach end of the box limits
                if ((textOffsetX + glyphWidth) > textBounds.width - textBoundsWidthOffset)
                {
                    textOffsetX = 0.0f;
                    textOffsetY += GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);

                    if (tempWrapCharMode)   // Wrap at char level when too long words
                    {
                        wrapMode = TEXT_WRAP_WORD;
                        tempWrapCharMode = false;
                    }
                }
            }
            else if (wrapMode == TEXT_WRAP_WORD)
            {
                if (codepoint == 32) lastSpaceIndex = c;

                // Get width to next space in line
                int nextSpaceIndex = 0;
                float nextSpaceWidth = GetNextSpaceWidth(lines[i] + c, &nextSpaceIndex);

                int nextSpaceIndex2 = 0;
                float nextWordSize = GetNextSpaceWidth(lines[i] + lastSpaceIndex + 1, &nextSpaceIndex2);

                if (nextWordSize > textBounds.width - textBoundsWidthOffset)
                {
                    // Considering the case the next word is longer than bounds
                    tempWrapCharMode = true;
                    wrapMode = TEXT_WRAP_CHAR;
                }
                else if ((textOffsetX + nextSpaceWidth) > textBounds.width - textBoundsWidthOffset)
                {
                    textOffsetX = 0.0f;
                    textOffsetY += GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);
                }
            }

            if (codepoint == '\n') break;   // WARNING: Lines are already processed manually, no need to keep drawing after this codepoint
            else
            {
                // TODO: There are multiple types of spaces in Unicode,
                // maybe it's a good idea to add support for more: http://jkorpela.fi/chars/spaces.html
                if ((codepoint != ' ') && (codepoint != '\t'))      // Do not draw codepoints with no glyph
                {
                    if (wrapMode == TEXT_WRAP_NONE)
                    {
                        // Draw only required text glyphs fitting the textBounds.width
                        if (textSizeX > textBounds.width)
                        {
                            if (textOffsetX <= (textBounds.width - glyphWidth - textBoundsWidthOffset - ellipsisWidth))
                            {
                                DrawTextCodepoint(guiFont, codepoint, RAYGUI_CLITERAL(Vector2){ textBoundsPosition.x + textOffsetX, textBoundsPosition.y + textOffsetY }, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), GuiFade(tint, guiAlpha));
                            }
                            else if (!textOverflow)
                            {
                                textOverflow = true;

                                for (int j = 0; j < ellipsisWidth; j += ellipsisWidth/3)
                                {
                                    DrawTextCodepoint(guiFont, '.', RAYGUI_CLITERAL(Vector2){ textBoundsPosition.x + textOffsetX + j, textBoundsPosition.y + textOffsetY }, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), GuiFade(tint, guiAlpha));
                                }
                            }
                        }
                        else
                        {
                            DrawTextCodepoint(guiFont, codepoint, RAYGUI_CLITERAL(Vector2){ textBoundsPosition.x + textOffsetX, textBoundsPosition.y + textOffsetY }, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), GuiFade(tint, guiAlpha));
                        }
                    }
                    else if ((wrapMode == TEXT_WRAP_CHAR) || (wrapMode == TEXT_WRAP_WORD))
                    {
                        // Draw only glyphs inside the bounds
                        if ((textBoundsPosition.y + textOffsetY) <= (textBounds.y + textBounds.height - GuiGetStyle(DEFAULT, TEXT_SIZE)))
                        {
                            DrawTextCodepoint(guiFont, codepoint, RAYGUI_CLITERAL(Vector2){ textBoundsPosition.x + textOffsetX, textBoundsPosition.y + textOffsetY }, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), GuiFade(tint, guiAlpha));
                        }
                    }
                }

                if (guiFont.glyphs[index].advanceX == 0) textOffsetX += ((float)guiFont.recs[index].width*scaleFactor + (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
                else textOffsetX += ((float)guiFont.glyphs[index].advanceX*scaleFactor + (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
            }
        }

        if (wrapMode == TEXT_WRAP_NONE) posOffsetY += (float)GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);
        else if ((wrapMode == TEXT_WRAP_CHAR) || (wrapMode == TEXT_WRAP_WORD)) posOffsetY += (textOffsetY + (float)GuiGetStyle(DEFAULT, TEXT_LINE_SPACING));
        //---------------------------------------------------------------------------------
    }

#if defined(RAYGUI_DEBUG_TEXT_BOUNDS)
    GuiDrawRectangle(textBounds, 0, WHITE, Fade(BLUE, 0.4f));
#endif
}


int GuiButtonEx(Rectangle bounds, const char *text) {
    int result = 0;
    GuiState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiControlExclusiveMode) {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = STATE_PRESSED;
            else state = STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) result = 1;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), GetColor(GuiGetStyle(BUTTON, BASE + (state*3))));
    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))));

    if (state == STATE_FOCUSED) GuiTooltip(bounds);
    //------------------------------------------------------------------

    return result;      // Button pressed: result = 1
}
*/
