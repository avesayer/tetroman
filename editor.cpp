// editor.cpp --- 
// 
// Filename: game.cpp
// Author: Sierra
// Created: Вт окт 10 10:32:14 2017 (+0300)
// Last-Updated: Пт окт 27 14:17:15 2017 (+0300)
//           By: Sierra
//

struct button_quad
{
    game_rect Quad;
    game_rect TextureQuad;
    game_texture *Texture;
};

static void
LevelEditorMakeTextButton(game_offscreen_buffer *Buffer, game_font *Font, char* Text,
                          s32 X, s32 Y, s32 Width, s32 Height, 
                          button_quad *ButtonQuad, u8 Red, u8 Green, u8 Blue)
{
    ButtonQuad->Quad.w  = Width;
    ButtonQuad->Quad.h  = Height;
    ButtonQuad->Quad.x  = X;
    ButtonQuad->Quad.y  = Y;
    
    GameMakeTextureFromString(ButtonQuad->Texture, Text, &ButtonQuad->TextureQuad, Font, {Red, Green, Blue}, Buffer);
    
    ButtonQuad->TextureQuad.w = (ButtonQuad->TextureQuad.w < ButtonQuad->Quad.w) ? ButtonQuad->TextureQuad.w : ButtonQuad->Quad.w;
    ButtonQuad->TextureQuad.h = (ButtonQuad->TextureQuad.h < ButtonQuad->Quad.h) ? ButtonQuad->TextureQuad.h : ButtonQuad->Quad.h;
    
    ButtonQuad->TextureQuad.x = ButtonQuad->Quad.x + (ButtonQuad->Quad.w / 2) - (ButtonQuad->TextureQuad.w / 2);
    ButtonQuad->TextureQuad.y = ButtonQuad->Quad.y + (ButtonQuad->Quad.h / 2) - (ButtonQuad->TextureQuad.h / 2);
}

static void
LevelEditorUpdateTextureOnButton(game_offscreen_buffer *Buffer, game_font *Font,
                                 char *Text, button_quad *ButtonQuad, 
                                 u8 Red, u8 Green, u8 Blue)
{
    GameMakeTextureFromString(ButtonQuad->Texture, Text, &ButtonQuad->TextureQuad, Font,{Red, Green, Blue}, Buffer);
    
    ButtonQuad->TextureQuad.w = (ButtonQuad->TextureQuad.w < ButtonQuad->Quad.w) ? ButtonQuad->TextureQuad.w : ButtonQuad->Quad.w;
    ButtonQuad->TextureQuad.h = (ButtonQuad->TextureQuad.h < ButtonQuad->Quad.h) ? ButtonQuad->TextureQuad.h : ButtonQuad->Quad.h;
    
    ButtonQuad->TextureQuad.x = ButtonQuad->Quad.x + (ButtonQuad->Quad.w / 2) - (ButtonQuad->TextureQuad.w / 2);
    ButtonQuad->TextureQuad.y = ButtonQuad->Quad.y + (ButtonQuad->Quad.h / 2) - (ButtonQuad->TextureQuad.h / 2);
    
}

static void
RenderButtonQuad(game_offscreen_buffer *Buffer, button_quad *ButtonQuad, 
                 u8 Red, u8 Green, u8 Blue, u8 Alpha)
{
    DEBUGRenderQuadFill(Buffer, &ButtonQuad->Quad, {Red, Green, Blue}, Alpha);
    DEBUGRenderQuad(Buffer, &ButtonQuad->Quad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, ButtonQuad->Texture, &ButtonQuad->TextureQuad);
}

struct label_button
{
    game_rect InfoQuad;
    game_rect InfoTextureQuad;
    game_texture *InfoTexture;
    
    game_rect MinusQuad;
    game_rect MinusTextureQuad;
    game_texture *MinusTexture;
    
    game_rect NumberQuad;
    game_rect NumberTextureQuad;
    game_texture *NumberTexture;
    
    game_rect PlusQuad;
    game_rect PlusTextureQuad;
    game_texture *PlusTexture;
};

enum cursor_type
{
    ARROW,
    
    SIZE_ALL,
    
    SIZE_WEST,
    SIZE_EAST,
    SIZE_NORTH,
    SIZE_SOUTH,
    
    SIZE_WN,
    SIZE_ES,
    SIZE_NE,
    SIZE_WS
};

enum coordinate_type
{
    PIXEL_AREA,
    GAME_AREA,
    SCREEN_AREA
};

struct position_panel
{
    coordinate_type CoordType;
    
    button_quad HeaderButton;
    
    button_quad LeftArrowButton;
    button_quad SwitchNameButton;
    button_quad RightArrowButton;
    
    button_quad FirstNumberNameButton;
    button_quad FirstNumberButton;
    
    button_quad SecondNumberNameButton;
    button_quad SecondNumberButton;
    
    button_quad ThirdNumberNameButton;
    button_quad ThirdNumberButton;
    
    button_quad FourthNumberNameButton;
    button_quad FourthNumberButton;
};

enum scale_type
{
    WIDTH,
    HEIGHT,
    BOTH
};

static scale_type
UpdateScaleType(scale_type ScaleType)
{
    scale_type Result;
    
    switch(ScaleType) 
    {
        case WIDTH:
        {
            Result = HEIGHT;
        } break;
        case HEIGHT:
        {
            Result = BOTH;
        } break;
        case BOTH:
        {
            Result = WIDTH;
        } break;
    }
    
    return(Result);
}

enum res_type
{
    TARGET_WIDTH,
    TARGET_HEIGHT,
    REF_WIDTH,
    REF_HEIGHT
};

struct resolution_routine
{
    /* Resolution Scaler button routine*/
    
    button_quad ScalerHeaderButton;
    
    button_quad LeftArrowButton;
    button_quad SwitchButton;
    button_quad RightArrowButton;
    
    /* Target resolution button routine */
    
    button_quad TargetResolutionHeaderButton;
    
    button_quad TargetWidthNameButton;
    button_quad TargetWidthNumberButton;
    
    button_quad TargetHeightNameButton;
    button_quad TargetHeightNumberButton;
    
    /* Reference resolution button routine */
    
    button_quad ReferenceResolutionHeaderButton;
    
    button_quad ReferenceWidthNameButton;
    button_quad ReferenceWidthButton;
    
    button_quad ReferenceHeightNameButton;
    button_quad ReferenceHeightButton;
    
    res_type ResolutionType;
    
    s32 ReferenceWidth;
    s32 ReferenceHeight;
    s32 TargetWidth;
    s32 TargetHeight;
    
    s32 ResOldNumber;
    s32 ResNumberBufferIndex;
    char ResNumberBuffer[5];
    b32 ResNumberSelected;
    
    button_quad ApplyButton;
};

struct editor_object
{
    scale_type ScaleType;
    game_rect AreaQuad;
};

enum editor_type
{
    LEVEL_EDITOR,
    GAME_EDITOR
};

struct level_editor
{
    /* For switching to editor mode*/
    b32 IsOn;
    
    /* For creating string textures*/
    game_font *Font;
    
    /* Data for selecting figures */
    s32 CurrentFigureIndex;
    s32 CurrentLevelIndex;
    
    /* Data for changing level number */
    s32 OldLevelNumber;
    s32 LevelNumberBufferIndex;
    char LevelNumberBuffer[4];
    bool LevelNumberSelected;
    
    /* Current level index data */
    game_rect LevelIndexQuad;
    game_texture *LevelIndexTexture;
    
    /* Current level number data */
    //game_rect LevelNumberQuad;
    //game_texture *LevelNumberTexture;
    
    /* Next and Previous button data*/
    game_rect PrevLevelQuad;
    game_texture *PrevLevelTexture;
    
    game_rect NextLevelQuad;
    game_texture *NextLevelTexture;
    
    /* For highlightning the button whenever pressed */
    bool ButtonSelected;
    game_rect HighlightButtonQuad;
    
    // Layer for switching between level and game confgiuration
    button_quad SwitchConfiguration;
    editor_type EditorType;
    
    /* Level properties layer quad */
    game_rect LevelPropertiesQuad;
    
    button_quad LevelConfigButton;
    
    label_button RowLabel;
    label_button ColumnLabel;
    label_button FigureLabel;
    
    /* Figure properties layer quad */
    
    game_rect FigurePropertiesQuad;
    
    button_quad FigureConfigButton;
    button_quad RotateFigureButton;
    button_quad FlipFigureButton;
    button_quad TypeFigureButton;
    button_quad FormFigureButton;
    
    /* IOproperties layer quad */
    
    game_rect IOPropertiesQuad;
    
    button_quad IOConfigButton;
    button_quad SaveLevelButton;
    button_quad LoadLevelButton;
    
    /* Resize/Positioning routine */
    bool ShiftKeyPressed;
    cursor_type CursorType;
    
    bool AreaIsMoving;
    
    /* Position panel routine*/
    game_rect PosPanelQuad;
    position_panel PosPanel;
    
    /* Resolution panel routine */
    game_rect ResPanelQuad;
    resolution_routine ResPanel;
    
    // Objects that we actually edit
    bool ObjectIsSelected;
    s32 EditorObjectIndex;
    editor_object EditorObject[2];
};

static void
LevelEditorUpdateTextOnButton(game_offscreen_buffer *Buffer, game_font *&Font, char *TextBuffer, game_texture *&Texture, game_rect *TextureQuad, game_rect *AreaQuad, game_color Color)
{
    GameMakeTextureFromString(Texture, TextBuffer, TextureQuad, Font, Color, Buffer);
    
    TextureQuad->x = AreaQuad->x + (AreaQuad->w / 2) - (TextureQuad->w / 2);
    TextureQuad->y = AreaQuad->y + (AreaQuad->h / 2) - (TextureQuad->h / 2);
}


static void
LevelEditorUpdateCoordinates(game_offscreen_buffer *Buffer, game_font *Font, position_panel *PosPanel, game_rect Rectangle, 
                             game_rect GameArea, game_rect ScreenArea)
{
    char FirstNumberBuffer[8]  = {};
    char SecondNumberBuffer[8] = {};
    char ThirdNumberBuffer[8]  = {};
    char FourthNumberBuffer[8] = {};
    
    if(PosPanel->CoordType == PIXEL_AREA)
    {
        sprintf(FirstNumberBuffer,  "%d", Rectangle.x);
        sprintf(SecondNumberBuffer, "%d", Rectangle.y);
        sprintf(ThirdNumberBuffer,  "%d", Rectangle.w);
        sprintf(FourthNumberBuffer, "%d", Rectangle.h);
        
    }
    else if(PosPanel->CoordType == GAME_AREA)
    {
        math_rect MathRect = {
            (r32)Rectangle.x, (r32)Rectangle.y, 
            (r32)Rectangle.x + (r32)Rectangle.w, (r32)Rectangle.y + (r32)Rectangle.h};
        
        math_rect RelMathRect = {(r32)GameArea.x, (r32)GameArea.y, (r32)GameArea.x + (r32)GameArea.w, (r32)GameArea.y + (r32)GameArea.h};
        
        MathRect = NormalizeRectangle(MathRect, RelMathRect);
        
        sprintf(FirstNumberBuffer,  "%.3f", MathRect.Left);
        sprintf(SecondNumberBuffer, "%.3f", MathRect.Top);
        sprintf(ThirdNumberBuffer,  "%.3f", MathRect.Right);
        sprintf(FourthNumberBuffer, "%.3f", MathRect.Bottom);
    }
    else if(PosPanel->CoordType == SCREEN_AREA)
    {
        math_rect MathRect = {
            (r32)Rectangle.x, (r32)Rectangle.y, 
            (r32)Rectangle.x + (r32)Rectangle.w, (r32)Rectangle.y + (r32)Rectangle.h};
        
        math_rect RelMathRect = {(r32)ScreenArea.x, (r32)ScreenArea.y, (r32)ScreenArea.x + (r32)ScreenArea.w, (r32)ScreenArea.y + (r32)ScreenArea.h};
        
        MathRect = NormalizeRectangle(MathRect, RelMathRect);
        
        sprintf(FirstNumberBuffer,  "%.3f", MathRect.Left);
        sprintf(SecondNumberBuffer, "%.3f", MathRect.Top);
        sprintf(ThirdNumberBuffer,  "%.3f", MathRect.Right);
        sprintf(FourthNumberBuffer, "%.3f", MathRect.Bottom);
    }
    
    LevelEditorUpdateTextOnButton(Buffer, Font, FirstNumberBuffer, PosPanel->FirstNumberButton.Texture, &PosPanel->FirstNumberButton.TextureQuad, &PosPanel->FirstNumberButton.Quad, {255, 255, 255});
    
    LevelEditorUpdateTextOnButton(Buffer, Font, SecondNumberBuffer, PosPanel->SecondNumberButton.Texture, &PosPanel->SecondNumberButton.TextureQuad, &PosPanel->SecondNumberButton.Quad, {255, 255, 255});
    
    LevelEditorUpdateTextOnButton(Buffer, Font, ThirdNumberBuffer, PosPanel->ThirdNumberButton.Texture, &PosPanel->ThirdNumberButton.TextureQuad, &PosPanel->ThirdNumberButton.Quad, {255, 255, 255});
    
    LevelEditorUpdateTextOnButton(Buffer, Font, FourthNumberBuffer, PosPanel->FourthNumberButton.Texture, &PosPanel->FourthNumberButton.TextureQuad, &PosPanel->FourthNumberButton.Quad, {255, 255, 255});
}

static void
LevelEditorUpdateCoordinateSwitch(level_editor *LevelEditor, position_panel *PosPanel, game_offscreen_buffer *Buffer)
{
    coordinate_type CoordType = PosPanel->CoordType;
    if(CoordType == PIXEL_AREA)
    {
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Pixels", LevelEditor->PosPanel.SwitchNameButton.Texture, &LevelEditor->PosPanel.SwitchNameButton.TextureQuad, &LevelEditor->PosPanel.SwitchNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "X", LevelEditor->PosPanel.FirstNumberNameButton.Texture, &LevelEditor->PosPanel.FirstNumberNameButton.TextureQuad, &LevelEditor->PosPanel.FirstNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Y", LevelEditor->PosPanel.SecondNumberNameButton.Texture, &LevelEditor->PosPanel.SecondNumberNameButton.TextureQuad, &LevelEditor->PosPanel.SecondNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Width", LevelEditor->PosPanel.ThirdNumberNameButton.Texture, &LevelEditor->PosPanel.ThirdNumberNameButton.TextureQuad, &LevelEditor->PosPanel.ThirdNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Height", LevelEditor->PosPanel.FourthNumberNameButton.Texture, &LevelEditor->PosPanel.FourthNumberNameButton.TextureQuad, &LevelEditor->PosPanel.FourthNumberNameButton.Quad, {255, 255, 255});
    }
    else if(CoordType == GAME_AREA || CoordType == SCREEN_AREA)
    {
        if(CoordType == GAME_AREA)
        {
            LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "% game_area", LevelEditor->PosPanel.SwitchNameButton.Texture, &LevelEditor->PosPanel.SwitchNameButton.TextureQuad, &LevelEditor->PosPanel.SwitchNameButton.Quad, {255, 255, 255});
        }
        else
        {
            LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "% screen_area", LevelEditor->PosPanel.SwitchNameButton.Texture, &LevelEditor->PosPanel.SwitchNameButton.TextureQuad, &LevelEditor->PosPanel.SwitchNameButton.Quad, {255, 255, 255});
        }
        
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Left", LevelEditor->PosPanel.FirstNumberNameButton.Texture, &LevelEditor->PosPanel.FirstNumberNameButton.TextureQuad, &LevelEditor->PosPanel.FirstNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Top", LevelEditor->PosPanel.SecondNumberNameButton.Texture, &LevelEditor->PosPanel.SecondNumberNameButton.TextureQuad, &LevelEditor->PosPanel.SecondNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Right", LevelEditor->PosPanel.ThirdNumberNameButton.Texture, &LevelEditor->PosPanel.ThirdNumberNameButton.TextureQuad, &LevelEditor->PosPanel.ThirdNumberNameButton.Quad, {255, 255, 255});
        
        LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Bottom", LevelEditor->PosPanel.FourthNumberNameButton.Texture, &LevelEditor->PosPanel.FourthNumberNameButton.TextureQuad, &LevelEditor->PosPanel.FourthNumberNameButton.Quad, {255, 255, 255});
    }
    
}


static void
LevelEditorSetCursorType(cursor_type CursorType)
{
    SDL_Cursor* Cursor;
    
    switch(CursorType)
    {
        case ARROW:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        } break;
        
        case SIZE_ALL:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        } break;
        
        case SIZE_WEST:
        case SIZE_EAST:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        } break;
        
        case SIZE_NORTH:
        case SIZE_SOUTH:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
        } break;
        
        case SIZE_WN:
        case SIZE_ES:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
        } break;
        
        case SIZE_NE:
        case SIZE_WS:
        {
            Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
        } break;
    }
    
    SDL_SetCursor(Cursor);
}

static void 
LevelEditorUpdateLevelStats(s32 X, s32 Y, level_editor *LevelEditor, 
                            s32 LevelNumber, s32 LevelIndex, game_offscreen_buffer *Buffer)
{
    game_surface *Surface = {};
    
    char LevelIndexString[3]  = {};
    
    sprintf(LevelIndexString, "%d", LevelIndex);
    
    /* Level Index Texture initialization */
    
    {
        char TmpBuffer[128] = {};
        strcpy(TmpBuffer, "level index      = ");
        strcat(TmpBuffer, LevelIndexString);
        
        if(LevelEditor->LevelIndexTexture)
        {
            SDL_DestroyTexture(LevelEditor->LevelIndexTexture);
        }
        
        Surface = TTF_RenderUTF8_Blended(LevelEditor->Font, TmpBuffer, {255, 255, 255});
        Assert(Surface);
        
        LevelEditor->LevelIndexQuad.w = Surface->w;
        LevelEditor->LevelIndexQuad.h = Surface->h;
        LevelEditor->LevelIndexQuad.x = X;
        LevelEditor->LevelIndexQuad.y = Y;
        
        LevelEditor->LevelIndexTexture = SDL_CreateTextureFromSurface(Buffer->Renderer, Surface);
        Assert(LevelEditor->LevelIndexTexture);
        
        SDL_FreeSurface(Surface);
        
    }
}

static void
LevelEditorInitLabel(level_editor *LevelEditor, label_button *Label, char* Text,
                     s32 Number, s32 X, s32 Y, s32 LabelWidth, s32 LabelHeight, 
                     s32 InfoWidth, s32 BoxWidth, game_offscreen_buffer *Buffer)

{
    Label->InfoQuad = {X, Y, InfoWidth, LabelHeight};
    
    MenuMakeTextButton(Text, Label->InfoQuad.x, Label->InfoQuad.y, Label->InfoQuad.w,
                       Label->InfoQuad.h, &Label->InfoQuad, &Label->InfoTextureQuad,
                       Label->InfoTexture, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    Label->MinusQuad = {Label->InfoQuad.x + InfoWidth, Y, BoxWidth, LabelHeight};
    MenuMakeTextButton("-", Label->MinusQuad.x, Label->MinusQuad.y,
                       Label->MinusQuad.w, Label->MinusQuad.h, &Label->MinusQuad,
                       &Label->MinusTextureQuad, Label->MinusTexture, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    
    char NumberString[3] = {};
    sprintf(NumberString, "%d", Number);
    
    Label->NumberQuad = {Label->MinusQuad.x + BoxWidth, Y, BoxWidth, LabelHeight};
    MenuMakeTextButton(NumberString, Label->NumberQuad.x, Label->NumberQuad.y,
                       Label->NumberQuad.w, Label->NumberQuad.h, &Label->NumberQuad,
                       &Label->NumberTextureQuad, Label->NumberTexture, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    Label->PlusQuad = {Label->NumberQuad.x + BoxWidth, Y, BoxWidth, LabelHeight};
    MenuMakeTextButton("+", Label->PlusQuad.x, Label->PlusQuad.y,
                       Label->PlusQuad.w, Label->PlusQuad.h, &Label->PlusQuad,
                       &Label->PlusTextureQuad, Label->PlusTexture, LevelEditor->Font, {255, 255, 255}, Buffer);
    
}

static void
LevelEditorRenderLabel(label_button *Label, game_offscreen_buffer *Buffer)
{
    /* Information quad */
    DEBUGRenderQuadFill(Buffer, &Label->InfoQuad, {255, 0, 0}, 100);
    DEBUGRenderQuad(Buffer, &Label->InfoQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, Label->InfoTexture, &Label->InfoTextureQuad);
    
    /* Minus quad */
    DEBUGRenderQuadFill(Buffer, &Label->MinusQuad, {0, 255, 0}, 100);
    DEBUGRenderQuad(Buffer, &Label->MinusQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, Label->MinusTexture, &Label->MinusTextureQuad);
    
    /* Number quad */
    DEBUGRenderQuadFill(Buffer, &Label->NumberQuad, {0, 255, 0}, 100);
    DEBUGRenderQuad(Buffer, &Label->NumberQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, Label->NumberTexture, &Label->NumberTextureQuad);
    
    /* Plus quad */
    DEBUGRenderQuadFill(Buffer, &Label->PlusQuad, {0, 255, 0}, 100);
    DEBUGRenderQuad(Buffer, &Label->PlusQuad, {0, 0, 0}, 255);
    GameRenderBitmapToBuffer(Buffer, Label->PlusTexture, &Label->PlusTextureQuad);
}


static void
LevelEditorUpdatePositions(game_offscreen_buffer *Buffer, level_editor *LevelEditor, level_entity *LevelEntity, game_memory *Memory)
{
    //
    // Reference dimension assumed to be 800x600
    //
    
    s32 ScreenWidth     = Buffer->Width;
    s32 ScreenHeight    = Buffer->Height;
    s32 ReferenceWidth  = Buffer->ReferenceWidth;
    s32 ReferenceHeight = Buffer->ReferenceHeight;
    
    // New Font size
    {
        s32 FontSize = 12;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        if(LevelEditor->Font)
        {
            TTF_CloseFont(LevelEditor->Font);
        }
        
        FontSize = (r32)FontSize * NewScale;
        printf("FontSize = %d\n",FontSize);
        
        LevelEditor->Font = TTF_OpenFont("..\\data\\Karmina-Bold.otf", FontSize);
        Assert(LevelEditor->Font);
        
    }
    
    // New Game area location
    
    {
        Memory->PadRect.x = 40;
        Memory->PadRect.y = 30;
        Memory->PadRect.w = 720;
        Memory->PadRect.h = 570;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        Memory->PadRect.w = (r32)Memory->PadRect.w * NewScale;
        Memory->PadRect.h = (r32)Memory->PadRect.h * NewScale;
        Memory->PadRect.x = (r32)Memory->PadRect.x * NewScale;
        Memory->PadRect.y = (r32)Memory->PadRect.y * NewScale;
    }
    
    {
        game_rect GridArea = {};//475;
        
        GridArea.w = 720;
        GridArea.h = 390;
        GridArea.x = 40;
        GridArea.y = 30;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        GridArea.w = (r32)GridArea.w * NewScale;
        GridArea.h = (r32)GridArea.h * NewScale;
        GridArea.x = (r32)GridArea.x * NewScale;
        GridArea.y = (r32)GridArea.y * NewScale;
        
        LevelEntity->GridEntity->GridArea = GridArea;
        LevelEditor->EditorObject[0].AreaQuad = GridArea;
        
        LevelEntity->Configuration.GridBlockSize = CalculateGridBlockSize(LevelEntity->GridEntity->RowAmount, LevelEntity->GridEntity->ColumnAmount, GridArea.w, GridArea.h);
        
    }
    
    /* Next/Prev level buttons */
    
    { 
        LevelEditor->PrevLevelQuad.w = 40;
        LevelEditor->PrevLevelQuad.h = 40;
        
        LevelEditor->NextLevelQuad.w = 40;
        LevelEditor->NextLevelQuad.h = 40;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        LevelEditor->PrevLevelQuad.w = (r32)LevelEditor->PrevLevelQuad.w * NewScale;
        LevelEditor->PrevLevelQuad.h = (r32)LevelEditor->PrevLevelQuad.h * NewScale;
        LevelEditor->PrevLevelQuad.x = Memory->PadRect.x; LevelEditor->PrevLevelQuad.y = Memory->PadRect.y - LevelEditor->PrevLevelQuad.h; 
        
        LevelEditor->NextLevelQuad.w = (r32)LevelEditor->NextLevelQuad.w * NewScale;
        LevelEditor->NextLevelQuad.h = (r32)LevelEditor->NextLevelQuad.h * NewScale;
        LevelEditor->NextLevelQuad.x = (Memory->PadRect.x + Memory->PadRect.w) - LevelEditor->NextLevelQuad.w;
        LevelEditor->NextLevelQuad.y = Memory->PadRect.y + Memory->PadRect.h - LevelEditor->NextLevelQuad.h;
        
    }
    
    // Level Properties location
    
    {
        s32 ButtonWidth  = 150;
        s32 ButtonHeight = 30;
        s32 ButtonAmount = 4;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        ButtonWidth  = roundf((r32)ButtonWidth  * NewScale);
        ButtonHeight = roundf((r32)ButtonHeight * NewScale);
        
        LevelEditor->LevelPropertiesQuad.x = 0;
        LevelEditor->LevelPropertiesQuad.y = 0;
        LevelEditor->LevelPropertiesQuad.w = ButtonWidth;
        LevelEditor->LevelPropertiesQuad.h = ButtonHeight * ButtonAmount;
        
        /* Level header name initialization */
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Level Configuration", 
                                  LevelEditor->LevelPropertiesQuad.x,
                                  LevelEditor->LevelPropertiesQuad.y,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->SwitchConfiguration, 255, 255, 255);
        
        s32 InfoWidth = 75;
        s32 BoxWidth  = 25;
        
        InfoWidth = roundf((r32)InfoWidth * NewScale);
        BoxWidth  = roundf((r32)BoxWidth  * NewScale);
        
        /* Row label initialization */
        
        s32 RowAmount = LevelEntity->GridEntity->RowAmount;
        LevelEditorInitLabel(LevelEditor, &LevelEditor->RowLabel, "Row amount",
                             RowAmount, 0, ButtonHeight, ButtonWidth, ButtonHeight,
                             InfoWidth, BoxWidth, Buffer);
        
        /* Column label initialization */
        
        s32 ColumnAmount = LevelEntity->GridEntity->ColumnAmount;
        LevelEditorInitLabel(LevelEditor, &LevelEditor->ColumnLabel, "Column amount",
                             ColumnAmount, 0, ButtonHeight*2, ButtonWidth, ButtonHeight, InfoWidth, BoxWidth, Buffer);
        
        /* Figure label initialization */
        
        s32 FigureAmount = LevelEntity->FigureEntity->FigureAmount;
        LevelEditorInitLabel(LevelEditor, &LevelEditor->FigureLabel, "Figure amount",
                             FigureAmount, 0, ButtonHeight*3, ButtonWidth, ButtonHeight, InfoWidth, BoxWidth, Buffer);
    }
    
    // Figure Properties location
    
    {
        s32 ButtonWidth  = 150;
        s32 ButtonHeight = 30;
        s32 ButtonAmount = 5;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        ButtonWidth  = roundf((r32)ButtonWidth  * NewScale);
        ButtonHeight = roundf((r32)ButtonHeight * NewScale); 
        
        LevelEditor->FigurePropertiesQuad.w = ButtonWidth;
        LevelEditor->FigurePropertiesQuad.h = ButtonHeight * ButtonAmount;
        LevelEditor->FigurePropertiesQuad.x = LevelEditor->LevelPropertiesQuad.x;
        LevelEditor->FigurePropertiesQuad.y = LevelEditor->LevelPropertiesQuad.y
            + LevelEditor->LevelPropertiesQuad.h;
        
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Figure Configuration", 
                                  LevelEditor->FigurePropertiesQuad.x,
                                  LevelEditor->FigurePropertiesQuad.y,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->FigureConfigButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Flip figure", 
                                  LevelEditor->FigurePropertiesQuad.x,
                                  LevelEditor->FigurePropertiesQuad.y + ButtonHeight,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->FlipFigureButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Rotate figure", 
                                  LevelEditor->FigurePropertiesQuad.x,
                                  LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 2),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->RotateFigureButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Change type", 
                                  LevelEditor->FigurePropertiesQuad.x,
                                  LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 3),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->TypeFigureButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Change form", 
                                  LevelEditor->FigurePropertiesQuad.x,
                                  LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 4),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->FormFigureButton, 255, 255, 255);
        
    }
    
    // IO properties location
    
    {
        s32 ButtonWidth  = 150;
        s32 ButtonHeight = 30;
        s32 ButtonAmount = 3;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        ButtonWidth  = roundf((r32)ButtonWidth  * NewScale);
        ButtonHeight = roundf((r32)ButtonHeight * NewScale); 
        
        LevelEditor->IOPropertiesQuad.w = ButtonWidth;
        LevelEditor->IOPropertiesQuad.h = ButtonHeight * ButtonAmount;
        LevelEditor->IOPropertiesQuad.x = LevelEditor->FigurePropertiesQuad.x;
        LevelEditor->IOPropertiesQuad.y = LevelEditor->FigurePropertiesQuad.y + LevelEditor->FigurePropertiesQuad.h;
        
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Save/Load level", 
                                  LevelEditor->IOPropertiesQuad.x,
                                  LevelEditor->IOPropertiesQuad.y,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->IOConfigButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Save to disk", 
                                  LevelEditor->IOPropertiesQuad.x,
                                  LevelEditor->IOPropertiesQuad.y + (ButtonHeight),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->SaveLevelButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Load from disk", 
                                  LevelEditor->IOPropertiesQuad.x,
                                  LevelEditor->IOPropertiesQuad.y + (ButtonHeight * 2),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->LoadLevelButton, 255, 255, 255);
        
    }
    
    // Level stats location
    {
        s32 PosX = LevelEditor->LevelPropertiesQuad.x
            + LevelEditor->LevelPropertiesQuad.w + 10;
        s32 PosY = 0;
        
        LevelEditorUpdateLevelStats(PosX, PosY, LevelEditor, 0, Memory->CurrentLevelIndex, Buffer);
    }
    
    
    // Position Panel location
    
    {
        
        s32 ButtonWidth  = 150;
        s32 ButtonHeight = 20;
        s32 ButtonAmount = 6;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        ButtonWidth  = roundf((r32) ButtonWidth * NewScale);
        ButtonHeight = roundf((r32) ButtonHeight * NewScale);
        
        LevelEditor->PosPanelQuad.x = 0;
        LevelEditor->PosPanelQuad.y = LevelEditor->LevelPropertiesQuad.y + LevelEditor->SwitchConfiguration.Quad.h;
        LevelEditor->PosPanelQuad.w = ButtonWidth;
        LevelEditor->PosPanelQuad.h = ButtonHeight * ButtonAmount;
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Position properties", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.HeaderButton, 255, 255, 255);
        
        s32 ArrowWidth  = 32;
        s32 SwitchWidth = 86;
        
        ArrowWidth  = roundf((r32)ArrowWidth * NewScale);
        SwitchWidth = roundf((r32)SwitchWidth * NewScale);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "<", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y + ButtonHeight,
                                  ArrowWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.LeftArrowButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Pixels", 
                                  LevelEditor->PosPanelQuad.x + ArrowWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight),
                                  SwitchWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.SwitchNameButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ">", 
                                  LevelEditor->PosPanelQuad.x + ArrowWidth + SwitchWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight),
                                  ArrowWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.RightArrowButton, 255, 255, 255);
        
        s32 NameNumberWidth = 60;
        s32 NumberWidth     = 90;
        
        NameNumberWidth = roundf((r32)NameNumberWidth * NewScale);
        NumberWidth     = roundf((r32)NumberWidth * NewScale);
        
        char NumberString[128] = {};
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "X", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 2),
                                  NameNumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.FirstNumberNameButton, 255, 255, 255);
        
        sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.x);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                                  LevelEditor->PosPanelQuad.x + NameNumberWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 2),
                                  NumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.FirstNumberButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Y", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 3),
                                  NameNumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.SecondNumberNameButton, 255, 255, 255);
        
        sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.y);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                                  LevelEditor->PosPanelQuad.x + NameNumberWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 3),
                                  NumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.SecondNumberButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 4),
                                  NameNumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.ThirdNumberNameButton, 255, 255, 255);
        sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.w);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                                  LevelEditor->PosPanelQuad.x + NameNumberWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 4),
                                  NumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.ThirdNumberButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                                  LevelEditor->PosPanelQuad.x,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 5),
                                  NameNumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.FourthNumberNameButton, 255, 255, 255);
        
        sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.h);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                                  LevelEditor->PosPanelQuad.x + NameNumberWidth,
                                  LevelEditor->PosPanelQuad.y + (ButtonHeight * 5),
                                  NumberWidth, ButtonHeight,
                                  &LevelEditor->PosPanel.FourthNumberButton, 255, 255, 255);
        
    }
    
    // Resolution panel location
    
    {
        s32 ButtonWidth  = 150;
        s32 ButtonHeight = 20;
        s32 ButtonAmount = 7;
        
        r32 NewScale = GetScale(ScreenWidth, ScreenHeight, ReferenceWidth, ReferenceHeight, 0.0f);
        
        ButtonWidth  = roundf((r32) ButtonWidth  * NewScale);
        ButtonHeight = roundf((r32) ButtonHeight * NewScale);
        
        LevelEditor->ResPanelQuad.x = LevelEditor->PosPanelQuad.x;
        LevelEditor->ResPanelQuad.y = LevelEditor->PosPanelQuad.y + LevelEditor->PosPanelQuad.h;
        LevelEditor->ResPanelQuad.w = ButtonWidth;
        LevelEditor->ResPanelQuad.h = ButtonHeight * ButtonAmount;
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Scale Factor", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y,
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.ScalerHeaderButton, 255, 255, 255);
        
        s32 ArrowWidth  = 30;
        s32 SwitchWidth = 90;
        ArrowWidth  = roundf((r32)ArrowWidth * NewScale);
        SwitchWidth = roundf((r32)SwitchWidth * NewScale);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "<", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + ButtonHeight,
                                  ArrowWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.LeftArrowButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                                  LevelEditor->ResPanelQuad.x + ArrowWidth,
                                  LevelEditor->ResPanelQuad.y + ButtonHeight,
                                  SwitchWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.SwitchButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ">", 
                                  LevelEditor->ResPanelQuad.x + ArrowWidth + SwitchWidth,
                                  LevelEditor->ResPanelQuad.y + ButtonHeight,
                                  ArrowWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.RightArrowButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Target Resolution", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 2),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.TargetResolutionHeaderButton, 255, 255, 255);
        
        s32 ButtonName  = 38;
        ButtonName = roundf((r32)ButtonName * NewScale);
        
        s32 ButtonValue = 37;
        ButtonValue = roundf((r32) ButtonValue * NewScale);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                                  ButtonName, ButtonHeight,
                                  &LevelEditor->ResPanel.TargetWidthNameButton, 255, 255, 255);
        
        char TargetWidthBuffer[8] = {};
        LevelEditor->ResPanel.TargetWidth = ScreenWidth;
        sprintf(TargetWidthBuffer, "%d", LevelEditor->ResPanel.TargetWidth);
        
        char TargetHeightBuffer[8] = {};
        LevelEditor->ResPanel.TargetHeight = ScreenHeight;
        sprintf(TargetHeightBuffer, "%d", LevelEditor->ResPanel.TargetHeight);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, TargetWidthBuffer, 
                                  LevelEditor->ResPanelQuad.x + ButtonName,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                                  ButtonValue, ButtonHeight,
                                  &LevelEditor->ResPanel.TargetWidthNumberButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                                  LevelEditor->ResPanelQuad.x + (ButtonName + ButtonValue),
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                                  ButtonName, ButtonHeight,
                                  &LevelEditor->ResPanel.TargetHeightNameButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, TargetHeightBuffer, 
                                  LevelEditor->ResPanelQuad.x + ((ButtonName * 2) + ButtonValue),
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                                  ButtonValue, ButtonHeight,
                                  &LevelEditor->ResPanel.TargetHeightNumberButton, 255, 255, 255);
        
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Reference Resolution", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 4),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.ReferenceResolutionHeaderButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                                  ButtonName, ButtonHeight,
                                  &LevelEditor->ResPanel.ReferenceWidthNameButton, 255, 255, 255);
        
        char ReferenceWidthBuffer[8] = {};
        LevelEditor->ResPanel.ReferenceWidth = Buffer->ReferenceWidth;
        sprintf(ReferenceWidthBuffer, "%d", LevelEditor->ResPanel.ReferenceWidth);
        
        char ReferenceHeightBuffer[8] = {};
        LevelEditor->ResPanel.ReferenceHeight = Buffer->ReferenceHeight;
        sprintf(ReferenceHeightBuffer, "%d", LevelEditor->ResPanel.ReferenceHeight);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ReferenceWidthBuffer, 
                                  LevelEditor->ResPanelQuad.x + ButtonName,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                                  ButtonValue, ButtonHeight,
                                  &LevelEditor->ResPanel.ReferenceWidthButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                                  LevelEditor->ResPanelQuad.x + (ButtonName + ButtonValue),
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                                  ButtonName, ButtonHeight,
                                  &LevelEditor->ResPanel.ReferenceHeightNameButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ReferenceHeightBuffer, 
                                  LevelEditor->ResPanelQuad.x + ((ButtonName * 2) + ButtonValue),
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                                  ButtonValue, ButtonHeight,
                                  &LevelEditor->ResPanel.ReferenceHeightButton, 255, 255, 255);
        
        LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Apply", 
                                  LevelEditor->ResPanelQuad.x,
                                  LevelEditor->ResPanelQuad.y + (ButtonHeight * 6),
                                  ButtonWidth, ButtonHeight,
                                  &LevelEditor->ResPanel.ApplyButton, 255, 255, 255);
        
    }
}

static void
LevelEditorInit(level_editor *LevelEditor, level_entity *LevelEntity, game_memory *Memory, game_offscreen_buffer *Buffer)
{
    *LevelEditor = {};
    
    LevelEditor->EditorType = LEVEL_EDITOR;
    
    LevelEditor->ObjectIsSelected  = false;
    LevelEditor->EditorObjectIndex = 0;
    
    // Grid Area Object
    LevelEditor->EditorObject[0].ScaleType = WIDTH;
    LevelEditor->EditorObject[0].AreaQuad  = LevelEntity->GridEntity->GridArea;
    
    // Figure Area Object
    LevelEditor->EditorObject[1].ScaleType = WIDTH;
    LevelEditor->EditorObject[1].AreaQuad  = LevelEntity->FigureEntity->FigureArea; 
    
    /* Next/Prev level buttons */
    
    LevelEditor->PrevLevelTexture = GetTexture(Memory, "left_arrow.png", Buffer->Renderer);
    Assert(LevelEditor->PrevLevelTexture);
    
    LevelEditor->NextLevelTexture = GetTexture(Memory, "right_arrow.png", Buffer->Renderer);
    Assert(LevelEditor->NextLevelTexture);
    
    LevelEditorUpdatePositions(Buffer, LevelEditor, LevelEntity, Memory);
    
#if 0
    r32 ButtonNormHeight = 0.053333f;
    r32 ButtonNormWidth  = 0.045;
    s32 ButtonWidth  = roundf(ButtonNormWidth * (r32)Buffer->Width);
    s32 ButtonHeight = roundf(ButtonNormHeight * (r32)Buffer->Height);
    
    math_rect ScreenArea = {0.0f, 0.0f, (r32)Buffer->Width, (r32)Buffer->Height};
    
    r32 NormalX = 0.0f;
    r32 NormalY = 1.0f;
    
    r32 NormalHeight = 1.0f - (ButtonNormHeight * 4.0f);
    r32 NormalWidth  = ButtonNormWidth * 4.0f;
    
    math_rect PropertiesQuad = CreateMathRect(NormalX, NormalY, NormalWidth, NormalHeight, ScreenArea);
    
    LevelEditor->LevelPropertiesQuad = ConvertMathRectToGameRect(PropertiesQuad);
    
    /* Level header name initialization */
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Level Configuration", 
                              LevelEditor->LevelPropertiesQuad.x,
                              LevelEditor->LevelPropertiesQuad.y,
                              LevelEditor->LevelPropertiesQuad.w, ButtonHeight,
                              &LevelEditor->SwitchConfiguration, 255, 255, 255);
    
    /* Row label initialization */
    
    s32 RowAmount = LevelEntity->GridEntity->RowAmount;
    LevelEditorInitLabel(LevelEditor, &LevelEditor->RowLabel, "Row amount",
                         RowAmount, 0, ButtonHeight, LevelEditor->LevelPropertiesQuad.w, ButtonHeight, Buffer);
    
    /* Column label initialization */
    
    s32 ColumnAmount = LevelEntity->GridEntity->ColumnAmount;
    LevelEditorInitLabel(LevelEditor, &LevelEditor->ColumnLabel, "Column amount",
                         ColumnAmount, 0, ButtonHeight*2, LevelEditor->LevelPropertiesQuad.w, ButtonHeight, Buffer);
    
    /* Figure label initialization */
    
    s32 FigureAmount = LevelEntity->FigureEntity->FigureAmount;
    LevelEditorInitLabel(LevelEditor, &LevelEditor->FigureLabel, "Figure amount",
                         FigureAmount, 0, ButtonHeight*3, LevelEditor->LevelPropertiesQuad.w, ButtonHeight, Buffer);
    
    
    /* Figure header name initialization */
    
    ButtonNormHeight = 0.05f;
    ButtonHeight = roundf(ButtonNormHeight * (r32)Buffer->Height);
    
    NormalY      = NormalHeight;
    NormalHeight = NormalY - ((r32)(ButtonHeight * 5) / (r32)(Buffer->Height));
    
    PropertiesQuad = CreateMathRect(NormalX, NormalY, NormalWidth, NormalHeight, ScreenArea);
    
    LevelEditor->FigurePropertiesQuad = ConvertMathRectToGameRect(PropertiesQuad);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Figure Configuration", 
                              LevelEditor->FigurePropertiesQuad.x,
                              LevelEditor->FigurePropertiesQuad.y,
                              LevelEditor->FigurePropertiesQuad.w, ButtonHeight,
                              &LevelEditor->FigureConfigButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Flip figure", 
                              LevelEditor->FigurePropertiesQuad.x,
                              LevelEditor->FigurePropertiesQuad.y + ButtonHeight,
                              LevelEditor->FigurePropertiesQuad.w, ButtonHeight,
                              &LevelEditor->FlipFigureButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Rotate figure", 
                              LevelEditor->FigurePropertiesQuad.x,
                              LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 2),
                              LevelEditor->FigurePropertiesQuad.w, ButtonHeight,
                              &LevelEditor->RotateFigureButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Change type", 
                              LevelEditor->FigurePropertiesQuad.x,
                              LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 3),
                              LevelEditor->FigurePropertiesQuad.w, ButtonHeight,
                              &LevelEditor->TypeFigureButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Change form", 
                              LevelEditor->FigurePropertiesQuad.x,
                              LevelEditor->FigurePropertiesQuad.y + (ButtonHeight * 4),
                              LevelEditor->FigurePropertiesQuad.w, ButtonHeight,
                              &LevelEditor->FormFigureButton, 255, 255, 255);
    
    /* IOproperties header name initialization */
    
    NormalY      = NormalHeight;
    NormalHeight = NormalY - ((r32)(ButtonHeight * 3) / (r32)(Buffer->Height));
    
    PropertiesQuad = CreateMathRect(NormalX, NormalY, NormalWidth, NormalHeight, ScreenArea);
    
    LevelEditor->IOPropertiesQuad = ConvertMathRectToGameRect(PropertiesQuad);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Save/Load level", 
                              LevelEditor->IOPropertiesQuad.x,
                              LevelEditor->IOPropertiesQuad.y,
                              LevelEditor->IOPropertiesQuad.w, ButtonHeight,
                              &LevelEditor->IOConfigButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Save to disk", 
                              LevelEditor->IOPropertiesQuad.x,
                              LevelEditor->IOPropertiesQuad.y + (ButtonHeight),
                              LevelEditor->IOPropertiesQuad.w, ButtonHeight,
                              &LevelEditor->SaveLevelButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Load from disk", 
                              LevelEditor->IOPropertiesQuad.x,
                              LevelEditor->IOPropertiesQuad.y + (ButtonHeight * 2),
                              LevelEditor->IOPropertiesQuad.w, ButtonHeight,
                              &LevelEditor->LoadLevelButton, 255, 255, 255);
    
    /* Level stats initialization */
    
    LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, 
                                LevelEditor, LevelEntity->LevelNumber, 
                                Memory->CurrentLevelIndex, Buffer);
    
    
    /* Position panel initialization */
    
    LevelEditor->PosPanel.CoordType = PIXEL_AREA;
    
    ButtonHeight = (r32)Buffer->Height * 0.04f;
    
    r32 PosPanelHeight = ButtonHeight * 6;
    r32 PosPanelWidth  = NormalWidth * (r32)Buffer->Width;//(r32)Buffer->Width * 0.19;
    
    r32 NormPanelWidth  = NormalWidth;
    r32 NormPanelHeight = PosPanelHeight / (r32) Buffer->Height;
    
    r32 PosPanelLeft   = 0;
    r32 PosPanelTop    = 1.0 - ButtonNormHeight;
    r32 PosPanelRight  = NormPanelWidth;
    r32 PosPanelBottom = PosPanelTop - (NormPanelHeight); 
    
    math_rect MathPanel = CreateMathRect(PosPanelLeft, PosPanelTop, PosPanelRight, PosPanelBottom, ScreenArea);
    
    LevelEditor->PosPanelQuad = ConvertMathRectToGameRect(MathPanel);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Position properties", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y,
                              LevelEditor->PosPanelQuad.w, ButtonHeight,
                              &LevelEditor->PosPanel.HeaderButton, 255, 255, 255);
    
    s32 ArrowWidth  = roundf((r32)PosPanelWidth * 0.2f);
    s32 SwitchWidth = roundf((r32)PosPanelWidth * 0.6f);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "<", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y + ButtonHeight,
                              ArrowWidth, ButtonHeight,
                              &LevelEditor->PosPanel.LeftArrowButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Pixels", 
                              LevelEditor->PosPanelQuad.x + ArrowWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight),
                              SwitchWidth, ButtonHeight,
                              &LevelEditor->PosPanel.SwitchNameButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ">", 
                              LevelEditor->PosPanelQuad.x + ArrowWidth + SwitchWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight),
                              ArrowWidth, ButtonHeight,
                              &LevelEditor->PosPanel.RightArrowButton, 255, 255, 255);
    
    s32 NameNumberWidth = roundf((r32)PosPanelWidth * 0.4f);
    s32 NumberWidth     = PosPanelWidth - NameNumberWidth;
    char NumberString[128] = {};
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "X", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 2),
                              NameNumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.FirstNumberNameButton, 255, 255, 255);
    
    sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.x);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                              LevelEditor->PosPanelQuad.x + NameNumberWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 2),
                              NumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.FirstNumberButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Y", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 3),
                              NameNumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.SecondNumberNameButton, 255, 255, 255);
    
    sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.y);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                              LevelEditor->PosPanelQuad.x + NameNumberWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 3),
                              NumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.SecondNumberButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 4),
                              NameNumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.ThirdNumberNameButton, 255, 255, 255);
    sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.w);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                              LevelEditor->PosPanelQuad.x + NameNumberWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 4),
                              NumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.ThirdNumberButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                              LevelEditor->PosPanelQuad.x,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 5),
                              NameNumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.FourthNumberNameButton, 255, 255, 255);
    
    sprintf(NumberString, "%d", LevelEntity->GridEntity->GridArea.h);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, NumberString, 
                              LevelEditor->PosPanelQuad.x + NameNumberWidth,
                              LevelEditor->PosPanelQuad.y + (ButtonHeight * 5),
                              NumberWidth, ButtonHeight,
                              &LevelEditor->PosPanel.FourthNumberButton, 255, 255, 255);
    
    
    /* Resolution panel initialization */
    
    LevelEditor->ResPanel.ReferenceWidth  = Buffer->Width;
    LevelEditor->ResPanel.ReferenceHeight = Buffer->Height;
    
    LevelEditor->ResPanel.TargetWidth  = Buffer->Width;
    LevelEditor->ResPanel.TargetHeight = Buffer->Height;
    
    r32 ResPanelWidth  = PosPanelWidth;
    r32 ResPanelHeight = 7 * ButtonHeight;
    
    NormPanelWidth  = ResPanelWidth  / (r32) Buffer->Width;
    NormPanelHeight = ResPanelHeight / (r32) Buffer->Height;
    
    r32 ResPanelLeft   = 0.0f;
    r32 ResPanelTop    = PosPanelBottom;
    r32 ResPanelRight  = NormPanelWidth;
    r32 ResPanelBottom = ResPanelTop - NormPanelHeight;
    
    math_rect ResolutionPanel = CreateMathRect(ResPanelLeft, ResPanelTop, ResPanelRight, ResPanelBottom, ScreenArea);
    
    LevelEditor->ResPanelQuad = ConvertMathRectToGameRect(ResolutionPanel);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Scale Factor", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y,
                              LevelEditor->ResPanelQuad.w, ButtonHeight,
                              &LevelEditor->ResPanel.ScalerHeaderButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "<", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + ButtonHeight,
                              ArrowWidth, ButtonHeight,
                              &LevelEditor->ResPanel.LeftArrowButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                              LevelEditor->ResPanelQuad.x + ArrowWidth,
                              LevelEditor->ResPanelQuad.y + ButtonHeight,
                              SwitchWidth, ButtonHeight,
                              &LevelEditor->ResPanel.SwitchButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ">", 
                              LevelEditor->ResPanelQuad.x + ArrowWidth + SwitchWidth,
                              LevelEditor->ResPanelQuad.y + ButtonHeight,
                              ArrowWidth, ButtonHeight,
                              &LevelEditor->ResPanel.RightArrowButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Target Resolution", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 2),
                              LevelEditor->ResPanelQuad.w, ButtonHeight,
                              &LevelEditor->ResPanel.TargetResolutionHeaderButton, 255, 255, 255);
    
    s32 ButtonQuad = LevelEditor->ResPanelQuad.w / 4;
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.TargetWidthNameButton, 255, 255, 255);
    
    char TargetWidthBuffer[8] = {};
    sprintf(TargetWidthBuffer, "%d", LevelEditor->ResPanel.TargetWidth);
    
    char TargetHeightBuffer[8] = {};
    sprintf(TargetHeightBuffer, "%d", LevelEditor->ResPanel.TargetHeight);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, TargetWidthBuffer, 
                              LevelEditor->ResPanelQuad.x + ButtonQuad,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.TargetWidthNumberButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                              LevelEditor->ResPanelQuad.x + (ButtonQuad * 2),
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.TargetHeightNameButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, TargetHeightBuffer, 
                              LevelEditor->ResPanelQuad.x + (ButtonQuad * 3),
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 3),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.TargetHeightNumberButton, 255, 255, 255);
    
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Reference Resolution", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 4),
                              LevelEditor->ResPanelQuad.w, ButtonHeight,
                              &LevelEditor->ResPanel.ReferenceResolutionHeaderButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Width", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.ReferenceWidthNameButton, 255, 255, 255);
    
    char ReferenceWidthBuffer[8] = {};
    sprintf(ReferenceWidthBuffer, "%d", LevelEditor->ResPanel.ReferenceWidth);
    
    char ReferenceHeightBuffer[8] = {};
    sprintf(ReferenceHeightBuffer, "%d", LevelEditor->ResPanel.ReferenceHeight);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ReferenceWidthBuffer, 
                              LevelEditor->ResPanelQuad.x + ButtonQuad,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.ReferenceWidthButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Height", 
                              LevelEditor->ResPanelQuad.x + (ButtonQuad * 2),
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.ReferenceHeightNameButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, ReferenceHeightBuffer, 
                              LevelEditor->ResPanelQuad.x + (ButtonQuad * 3),
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 5),
                              ButtonQuad, ButtonHeight,
                              &LevelEditor->ResPanel.ReferenceHeightButton, 255, 255, 255);
    
    LevelEditorMakeTextButton(Buffer, LevelEditor->Font, "Apply", 
                              LevelEditor->ResPanelQuad.x,
                              LevelEditor->ResPanelQuad.y + (ButtonHeight * 6),
                              LevelEditor->ResPanelQuad.w, ButtonHeight,
                              &LevelEditor->ResPanel.ApplyButton, 255, 255, 255);
    
#endif
    
}

static void
GridEntityDeleteMovingBlock(grid_entity *GridEntity, u32 Index)
{
    u32 Amount = GridEntity->MovingBlocksAmount;
    
    if(Index < 0 || Index >= Amount) return;
    if(Amount <= 0) return;
    
    moving_block *MovingBlocks = GridEntity->MovingBlocks;
    
    for(u32 i = Index; i < Amount-1; ++i)
    {
        MovingBlocks[i].AreaQuad   = MovingBlocks[i+1].AreaQuad;
        MovingBlocks[i].RowNumber  = MovingBlocks[i+1].RowNumber;
        MovingBlocks[i].ColNumber  = MovingBlocks[i+1].ColNumber;
        MovingBlocks[i].IsVertical = MovingBlocks[i+1].IsVertical;
        MovingBlocks[i].IsMoving   = MovingBlocks[i+1].IsMoving;
        MovingBlocks[i].MoveSwitch = MovingBlocks[i+1].MoveSwitch;
    }
    
    GridEntity->MovingBlocksAmount -= 1;
}

static void
LevelEditorChangeFigureCounter(level_editor *LevelEditor, s32 NewFigureAmount,
                               game_offscreen_buffer *Buffer)
{
    if(NewFigureAmount < 0)
    {
        return;
    }
    
    char FigureString[3] = {};
    
    sprintf(FigureString, "%d", NewFigureAmount);
    
    if(LevelEditor->FigureLabel.NumberTexture)
    {
        FreeTexture(LevelEditor->FigureLabel.NumberTexture);
        LevelEditor->FigureLabel.NumberTexture = 0;
    }
    
    GameMakeTextureFromString(LevelEditor->FigureLabel.NumberTexture, FigureString,
                              &LevelEditor->FigureLabel.NumberTextureQuad, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    LevelEditor->FigureLabel.NumberTextureQuad.x = LevelEditor->FigureLabel.NumberQuad.x + (LevelEditor->FigureLabel.NumberQuad.w / 2.0f)
        - (LevelEditor->FigureLabel.NumberTextureQuad.w / 2.0f);
    
    LevelEditor->FigureLabel.NumberTextureQuad.y = LevelEditor->FigureLabel.NumberQuad.y + (LevelEditor->FigureLabel.NumberQuad.h / 2.0f) - (LevelEditor->FigureLabel.NumberTextureQuad.h / 2.0f);
}

static void
LevelEditorChangeGridCounters(level_editor *LevelEditor, 
                              u32 NewRowAmount, u32 NewColumnAmount,
                              game_offscreen_buffer *Buffer)
{
    char RowString[3]   = {};
    char ColString[3]   = {};
    
    sprintf(RowString, "%d", NewRowAmount);
    sprintf(ColString, "%d", NewColumnAmount);
    
    if(LevelEditor->RowLabel.NumberTexture)
    {
        FreeTexture(LevelEditor->RowLabel.NumberTexture);
        LevelEditor->RowLabel.NumberTexture = 0;
    }
    
    GameMakeTextureFromString(LevelEditor->RowLabel.NumberTexture, RowString,
                              &LevelEditor->RowLabel.NumberTextureQuad, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    LevelEditor->RowLabel.NumberTextureQuad.x = LevelEditor->RowLabel.NumberQuad.x + (LevelEditor->RowLabel.NumberQuad.w / 2.0f)
        - (LevelEditor->RowLabel.NumberTextureQuad.w / 2.0f);
    LevelEditor->RowLabel.NumberTextureQuad.y = LevelEditor->RowLabel.NumberQuad.y + (LevelEditor->RowLabel.NumberQuad.h / 2.0f) - (LevelEditor->RowLabel.NumberTextureQuad.h / 2.0f);
    
    if(LevelEditor->ColumnLabel.NumberTexture)
    {
        FreeTexture(LevelEditor->ColumnLabel.NumberTexture);
        LevelEditor->ColumnLabel.NumberTexture = 0;
    }
    
    GameMakeTextureFromString(LevelEditor->ColumnLabel.NumberTexture, ColString,
                              &LevelEditor->ColumnLabel.NumberTextureQuad, LevelEditor->Font, {255, 255, 255}, Buffer);
    
    LevelEditor->ColumnLabel.NumberTextureQuad.x = LevelEditor->ColumnLabel.NumberQuad.x + (LevelEditor->ColumnLabel.NumberQuad.w / 2.0f)
        - (LevelEditor->ColumnLabel.NumberTextureQuad.w / 2.0f);
    LevelEditor->ColumnLabel.NumberTextureQuad.y = LevelEditor->ColumnLabel.NumberQuad.y + (LevelEditor->ColumnLabel.NumberQuad.h / 2.0f) - (LevelEditor->ColumnLabel.NumberTextureQuad.h / 2.0f);
}


static figure_form
LevelEditorGetNextFigureForm(figure_form CurrentForm)
{
    switch(CurrentForm)
    {
        case I_figure: return O_figure;
        case O_figure: return Z_figure;
        case Z_figure: return S_figure;
        case S_figure: return T_figure;
        case T_figure: return L_figure;
        case L_figure: return J_figure;
        case J_figure: return I_figure;
    }
    
    return O_figure;
}

static figure_type
LevelEditorGetNextFigureType(figure_type CurrentType)
{
    switch(CurrentType)
    {
        case classic: return stone;
        case stone:   return mirror;
        case mirror:  return classic;
    }
    
    return classic;
}


inline void
LevelEditorRenderButton(game_rect *ButtonQuad, game_rect *ImageQuad,
                        game_color ButtonColor, u8 Alpha, game_texture *ImageTexture,
                        s32 *Accumulator, s32 Offset,  game_offscreen_buffer *Buffer)
{
    DEBUGRenderQuad(Buffer, ButtonQuad, ButtonColor, Alpha);
    GameRenderBitmapToBuffer(Buffer, ImageTexture, ImageQuad);
    
    if(Accumulator)
    {
        *Accumulator += Offset;
    }
}

static void
GridEntityNewGrid(game_offscreen_buffer *Buffer, level_entity *LevelEntity, 
                  s32 NewRowAmount, s32 NewColumnAmount, level_editor *LevelEditor)
{
    if(NewRowAmount < 0 || NewColumnAmount < 0) return;
    
    grid_entity *&GridEntity = LevelEntity->GridEntity;
    
    s32 *UnitField = (s32*)calloc(NewRowAmount * NewColumnAmount, sizeof(s32));
    Assert(UnitField);
    for(u32 Row = 0; Row < NewRowAmount; ++Row){
        for(u32 Col = 0; Col < NewColumnAmount; Col++){
            UnitField[(Row * NewColumnAmount) + Col] = 0;
        }
    }
    
    u32 CurrentRowAmount = NewRowAmount < GridEntity->RowAmount ? NewRowAmount : GridEntity->RowAmount;
    u32 CurrentColumnAmount = NewColumnAmount < GridEntity->ColumnAmount ? NewColumnAmount : GridEntity->ColumnAmount;
    
    for(u32 Row = 0; Row < CurrentRowAmount; ++Row){
        for(u32 Col = 0; Col < CurrentColumnAmount; ++Col){
            s32 UnitIndex = (Row * CurrentColumnAmount) + Col;
            UnitField[UnitIndex] = GridEntity->UnitField[UnitIndex];
        }
    }
    
    free(GridEntity->UnitField);
    
    LevelEditorChangeGridCounters(LevelEditor, 
                                  NewRowAmount, NewColumnAmount, 
                                  Buffer);
    
    u32 DefaultBlocksInRow = 12;
    u32 DefaultBlocksInCol = 9;
    
    RescaleGameField(Buffer, NewRowAmount, NewColumnAmount,
                     LevelEntity->FigureEntity->FigureAmount, DefaultBlocksInRow, DefaultBlocksInCol, LevelEntity);
    
    GridEntity->UnitField    = UnitField;
    GridEntity->RowAmount    = NewRowAmount;
    GridEntity->ColumnAmount = NewColumnAmount;
    
    u32 ActiveBlockSize   = LevelEntity->Configuration.ActiveBlockSize;
    u32 InActiveBlockSize = LevelEntity->Configuration.InActiveBlockSize;
    
    LevelEntity->GridEntity->GridArea.w = ActiveBlockSize * NewColumnAmount;
    LevelEntity->GridEntity->GridArea.h = ActiveBlockSize * NewRowAmount;
    LevelEntity->GridEntity->GridArea.x = (Buffer->Width / 2) - (GridEntity->GridArea.w / 2);
    LevelEntity->GridEntity->GridArea.y = (Buffer->Height - LevelEntity->FigureEntity->FigureArea.h) / 2 - (GridEntity->GridArea.h / 2);
    
    for(u32 i = 0; i < LevelEntity->GridEntity->MovingBlocksAmount; ++i)
    {
        u32 RowNumber = LevelEntity->GridEntity->MovingBlocks[i].RowNumber;
        u32 ColNumber = LevelEntity->GridEntity->MovingBlocks[i].ColNumber;
        
        if(RowNumber >= NewRowAmount || ColNumber >= NewColumnAmount)
        {
            GridEntityDeleteMovingBlock(GridEntity, i);
        }
    }
    
    for(u32 i = 0; i < LevelEntity->GridEntity->MovingBlocksAmount; ++i)
    {
        u32 RowNumber = LevelEntity->GridEntity->MovingBlocks[i].RowNumber;
        u32 ColNumber = LevelEntity->GridEntity->MovingBlocks[i].ColNumber;
        
        GridEntity->MovingBlocks[i].AreaQuad.w = ActiveBlockSize;
        GridEntity->MovingBlocks[i].AreaQuad.h = ActiveBlockSize;
        GridEntity->MovingBlocks[i].AreaQuad.x = GridEntity->GridArea.x + (ColNumber * ActiveBlockSize);
        GridEntity->MovingBlocks[i].AreaQuad.y = GridEntity->GridArea.y + (RowNumber * ActiveBlockSize);
    }
    
}

inline static void
CleanResolutionNumber(resolution_routine *ResPanel, s32 OldNumber)
{
    printf("ResNumberSelected = true\n");
    
    ResPanel->ResNumberSelected = true;
    ResPanel->ResNumberBufferIndex = 0;
    ResPanel->ResOldNumber = OldNumber;
    
    ResPanel->ResNumberBuffer[0] = '\0';
    ResPanel->ResNumberBuffer[1] = '\0';
    ResPanel->ResNumberBuffer[2] = '\0';
    ResPanel->ResNumberBuffer[3] = '\0';
    ResPanel->ResNumberBuffer[4] = '\0';
}

static char
GetNumberFromInput(s32 BufferIndex, game_input *Input)
{
    char Result = '\n';
    
    s32 DigitIndex = BufferIndex;
    
    if(Input->Keyboard.Zero.EndedDown)
    {
        Result = '0';
    }
    else if(Input->Keyboard.One.EndedDown)
    {
        Result = '1';
    }
    else if(Input->Keyboard.Two.EndedDown)
    {
        Result = '2';
    }
    else if(Input->Keyboard.Three.EndedDown)
    {
        Result = '3';
    }
    else if(Input->Keyboard.Four.EndedDown)
    {
        Result = '4';
    }
    else if(Input->Keyboard.Five.EndedDown)
    {
        Result = '5';
    }
    else if(Input->Keyboard.Six.EndedDown)
    {
        Result = '6';
    }
    else if(Input->Keyboard.Seven.EndedDown)
    {
        Result = '7';
    }
    else if(Input->Keyboard.Eight.EndedDown)
    {
        Result = '8';
    }
    else if(Input->Keyboard.Nine.EndedDown)
    {
        Result = '9';
    }
    
    return (Result);
}

static void
GameConfigUpdateAndRender(level_editor *LevelEditor, level_entity *LevelEntity,
                          game_memory *Memory, game_offscreen_buffer *Buffer,
                          game_input *Input)
{
    s32 RowAmount = LevelEntity->GridEntity->RowAmount;
    s32 ColAmount = LevelEntity->GridEntity->ColumnAmount;
    
    game_rect GridArea = LevelEntity->GridEntity->GridArea;
    
    if(LevelEditor->ResPanel.ResNumberSelected)
    {
        s32 DigitIndex = LevelEditor->ResPanel.ResNumberBufferIndex;
        char NextDigit = GetNumberFromInput(DigitIndex, Input);
        
        if(NextDigit)
        {
            if(DigitIndex <= 3 && NextDigit != '\n')
            {
                LevelEditor->ResPanel.ResNumberBuffer[DigitIndex] = NextDigit;
                LevelEditor->ResPanel.ResNumberBufferIndex = ++DigitIndex;
                
                button_quad *CurrentButtonQuad;
                switch(LevelEditor->ResPanel.ResolutionType)
                {
                    case TARGET_WIDTH:
                    {
                        CurrentButtonQuad = &LevelEditor->ResPanel.TargetWidthNumberButton;
                    } break;
                    
                    case TARGET_HEIGHT:
                    {
                        CurrentButtonQuad = &LevelEditor->ResPanel.TargetHeightNumberButton;
                    } break;
                    
                    case REF_WIDTH:
                    {
                        CurrentButtonQuad = &LevelEditor->ResPanel.ReferenceWidthButton;
                    } break;
                    
                    case REF_HEIGHT:
                    {
                        CurrentButtonQuad = &LevelEditor->ResPanel.ReferenceHeightButton;
                    } break;
                }
                
                LevelEditorUpdateTextureOnButton(Buffer, LevelEditor->Font,
                                                 LevelEditor->ResPanel.ResNumberBuffer, CurrentButtonQuad, 
                                                 255, 255, 255);
            }
        }
    }
    
    if(Input->Keyboard.Enter.EndedDown)
    {
        if(LevelEditor->ResPanel.ResNumberSelected)
        {
            LevelEditor->ResPanel.ResNumberSelected = false;
            s32 ResultNumber = strtol(LevelEditor->ResPanel.ResNumberBuffer, 0, 10);
            
            switch(LevelEditor->ResPanel.ResolutionType)
            {
                case TARGET_WIDTH:
                {
                    LevelEditor->ResPanel.TargetWidth = ResultNumber;
                } break;
                
                case TARGET_HEIGHT:
                {
                    LevelEditor->ResPanel.TargetHeight = ResultNumber;
                } break;
                
                case REF_WIDTH:
                {
                    LevelEditor->ResPanel.ReferenceWidth = ResultNumber;
                } break;
                
                case REF_HEIGHT:
                {
                    LevelEditor->ResPanel.ReferenceHeight = ResultNumber;
                } break;
            }
        }
    }
    
    
    if(Input->Keyboard.LeftShift.EndedDown)
    {
        if(!LevelEditor->ShiftKeyPressed)
        {
            LevelEditor->ShiftKeyPressed = true;
            printf("Shift is pressed!\n");
        } 
    }
    else if(Input->Keyboard.LeftShift.EndedUp)
    {
        if(LevelEditor->ShiftKeyPressed)
        {
            LevelEditor->ShiftKeyPressed = false;
            printf("Shift is released!\n");
        }
    }
    
    if(Input->MouseButtons[0].EndedDown)
    {
        if(LevelEditor->ResPanel.ResNumberSelected)
        {
            LevelEditor->ResPanel.ResNumberSelected = false;
            
            char StringBuffer[4] = {};
            sprintf(StringBuffer, "%d", LevelEditor->ResPanel.ResOldNumber);
            
            button_quad *CurrentButtonQuad;
            res_type ResType = LevelEditor->ResPanel.ResolutionType;
            
            switch (ResType)
            {
                case TARGET_WIDTH:
                {
                    CurrentButtonQuad = &LevelEditor->ResPanel.TargetWidthNumberButton;
                } break;
                
                case TARGET_HEIGHT:
                {
                    CurrentButtonQuad = &LevelEditor->ResPanel.TargetHeightNumberButton;
                } break;
                
                case REF_WIDTH:
                {
                    CurrentButtonQuad = &LevelEditor->ResPanel.ReferenceWidthButton;
                } break;
                
                case REF_HEIGHT:
                {
                    CurrentButtonQuad = &LevelEditor->ResPanel.ReferenceHeightButton;
                } break;
            }
            
            LevelEditorUpdateTextureOnButton(Buffer, LevelEditor->Font,
                                             StringBuffer, CurrentButtonQuad, 
                                             255, 255, 255);
        }
        
        
        if(LevelEditor->CursorType != ARROW)
        {
            if(LevelEditor->ObjectIsSelected)
            {
                LevelEditor->AreaIsMoving = true;
            }
        }
        else
        {
            for(s32 i = 0; i < 2; ++i)
            {
                if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->EditorObject[i].AreaQuad))
                {
                    LevelEditor->ObjectIsSelected = true;
                    LevelEditor->EditorObjectIndex = i;
                }
            }
        }
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->SwitchConfiguration.Quad))
        {
            LevelEditor->EditorType = LEVEL_EDITOR;
            LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Level configuration", LevelEditor->SwitchConfiguration.Texture, &LevelEditor->SwitchConfiguration.TextureQuad, &LevelEditor->SwitchConfiguration.Quad, {255, 255, 255});
            
            LevelEditor->CursorType = ARROW;
            
            LevelEditor->ButtonSelected = true;
            LevelEditor->HighlightButtonQuad = LevelEditor->SwitchConfiguration.Quad;
        }
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->PosPanelQuad))
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->PosPanel.LeftArrowButton.Quad))
            {
                if(LevelEditor->PosPanel.CoordType == PIXEL_AREA)
                {
                    LevelEditor->PosPanel.CoordType = SCREEN_AREA;
                }
                else if(LevelEditor->PosPanel.CoordType == SCREEN_AREA)
                {
                    LevelEditor->PosPanel.CoordType = GAME_AREA;
                }
                else
                {
                    LevelEditor->PosPanel.CoordType = PIXEL_AREA;
                }
                
                LevelEditorUpdateCoordinates(Buffer, LevelEditor->Font, &LevelEditor->PosPanel, LevelEntity->GridEntity->GridArea, 
                                             Memory->PadRect, {0, 0, Buffer->Width, Buffer->Height});
                LevelEditorUpdateCoordinateSwitch(LevelEditor, &LevelEditor->PosPanel, Buffer);
                
                LevelEditor->ButtonSelected      = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->PosPanel.LeftArrowButton.Quad;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->PosPanel.RightArrowButton.Quad))
            {
                if(LevelEditor->PosPanel.CoordType == PIXEL_AREA)
                {
                    LevelEditor->PosPanel.CoordType = GAME_AREA;
                    
                }
                else if(LevelEditor->PosPanel.CoordType == GAME_AREA)
                {
                    LevelEditor->PosPanel.CoordType = SCREEN_AREA;
                }
                else
                {
                    LevelEditor->PosPanel.CoordType = PIXEL_AREA;
                }
                
                LevelEditorUpdateCoordinates(Buffer, LevelEditor->Font, &LevelEditor->PosPanel, LevelEntity->GridEntity->GridArea, 
                                             Memory->PadRect, {0, 0, Buffer->Width, Buffer->Height});
                LevelEditorUpdateCoordinateSwitch(LevelEditor, &LevelEditor->PosPanel, Buffer);
                
                LevelEditor->ButtonSelected      = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->PosPanel.RightArrowButton.Quad;
            }
            
        }
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanelQuad))
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.LeftArrowButton.Quad))
            {
                printf("Left arrow\n");
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->ResPanel.LeftArrowButton.Quad;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.RightArrowButton.Quad))
            {
                printf("Right arrow\n");
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->ResPanel.RightArrowButton.Quad;
            }
            
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.TargetWidthNumberButton.Quad))
            {
                printf("target width\n");
                
                CleanResolutionNumber(&LevelEditor->ResPanel, LevelEditor->ResPanel.TargetWidth);
                LevelEditor->ResPanel.ResolutionType = TARGET_WIDTH;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.TargetHeightNumberButton.Quad))
            {
                printf("target heigth\n");
                
                CleanResolutionNumber(&LevelEditor->ResPanel, LevelEditor->ResPanel.TargetHeight);
                
                LevelEditor->ResPanel.ResolutionType = TARGET_HEIGHT;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.ReferenceWidthButton.Quad))
            {
                printf("reference width\n");
                
                CleanResolutionNumber(&LevelEditor->ResPanel, LevelEditor->ResPanel.ReferenceWidth);
                
                LevelEditor->ResPanel.ResolutionType = REF_WIDTH;
                
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.ReferenceHeightButton.Quad))
            {
                printf("reference heigth\n");
                
                CleanResolutionNumber(&LevelEditor->ResPanel, LevelEditor->ResPanel.ReferenceHeight);
                
                LevelEditor->ResPanel.ResolutionType = REF_HEIGHT;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ResPanel.ApplyButton.Quad))
            {
                printf("apply button\n");
                printf("TargetWidth = %d\n", LevelEditor->ResPanel.TargetWidth);
                printf("TargetHeight = %d\n", LevelEditor->ResPanel.TargetHeight);
                printf("RefWidth = %d\n", LevelEditor->ResPanel.ReferenceWidth);
                printf("RefHeight = %d\n", LevelEditor->ResPanel.ReferenceHeight);
                
                Buffer->Width  = LevelEditor->ResPanel.TargetWidth;
                Buffer->Height = LevelEditor->ResPanel.TargetHeight;
                
                Buffer->ReferenceWidth = LevelEditor->ResPanel.ReferenceWidth;
                Buffer->ReferenceHeight = LevelEditor->ResPanel.ReferenceHeight;
                
                SDL_RenderSetLogicalSize(Buffer->Renderer, Buffer->Width, Buffer->Height);
                
                LevelEditorUpdatePositions(Buffer, LevelEditor, LevelEntity, Memory);
                GameUpdateRelativePositions(Buffer, LevelEntity, Memory);
                
            }
        }
        
    }
    else if(Input->MouseButtons[0].EndedUp)
    {
        LevelEditor->ButtonSelected      = true;
        LevelEditor->HighlightButtonQuad = {};
        
        if(LevelEditor->AreaIsMoving)
        {
            LevelEditor->CursorType = ARROW;
            LevelEditor->AreaIsMoving = false;
        }
    }
    
    if(LevelEditor->ObjectIsSelected && !LevelEditor->AreaIsMoving)
    {
        s32 Index = LevelEditor->EditorObjectIndex;
        game_rect AreaQuad = LevelEditor->EditorObject[Index].AreaQuad;
        
        game_rect Corner[4] = {};
        
        Corner[0] = {AreaQuad.x - 10, AreaQuad.y - 10, 20, 20};
        Corner[1] = {(AreaQuad.x + AreaQuad.w) - 10, AreaQuad.y - 10, 20, 20};
        Corner[2] = {(AreaQuad.x + AreaQuad.w) - 10, (AreaQuad.y + AreaQuad.h) - 10, 20, 20};
        Corner[3] = {AreaQuad.x - 10, (AreaQuad.y + AreaQuad.h), 20, 20};
        
        cursor_type CursorType = ARROW;
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Corner[0]))
        {
            CursorType = SIZE_WN;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Corner[1]))
        {
            CursorType = SIZE_NE;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Corner[2]))
        {
            CursorType = SIZE_ES;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Corner[3]))
        {
            CursorType = SIZE_WS;
        }
        else
        {
            game_rect Border[4] = {};
            
            Border[0] = {AreaQuad.x, AreaQuad.y - 10, AreaQuad.w, 20};
            Border[1] = {(AreaQuad.x + AreaQuad.w) - 10, AreaQuad.y, 20, AreaQuad.h};
            Border[2] = {AreaQuad.x, (AreaQuad.y + AreaQuad.h) - 10, AreaQuad.w, 20};
            Border[3] = {AreaQuad.x - 10, AreaQuad.y, 20, AreaQuad.h};
            
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Border[0]))
            {
                CursorType = SIZE_NORTH;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Border[1]))
            {
                CursorType = SIZE_EAST;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Border[2]))
            {
                CursorType = SIZE_SOUTH;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &Border[3]))
            {
                CursorType = SIZE_WEST;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &AreaQuad))
            {
                CursorType = SIZE_ALL;
            }
        }
        
        if(CursorType != LevelEditor->CursorType)
        {
            LevelEditor->CursorType = CursorType;
            LevelEditorSetCursorType(CursorType);
        }
    }
    
    if(LevelEditor->AreaIsMoving)
    {
        s32 OffsetX = Input->MouseRelX;
        s32 OffsetY = Input->MouseRelY;
        
        s32 Index = LevelEditor->EditorObjectIndex;
        
        math_rect AreaQuad = ConvertGameRectToMathRect(LevelEditor->EditorObject[Index].AreaQuad);
        
        switch(LevelEditor->CursorType)
        {
            case SIZE_ALL:
            {
                AreaQuad.Left   += OffsetX;
                AreaQuad.Top    += OffsetY;
                AreaQuad.Right  += OffsetX;
                AreaQuad.Bottom += OffsetY;
            } break;
            
            case SIZE_WEST:
            {
                AreaQuad.Left += OffsetX;
            } break;
            
            case SIZE_EAST:
            {
                AreaQuad.Right += OffsetX;
            } break;
            
            case SIZE_NORTH:
            {
                AreaQuad.Top += OffsetY;
            } break;
            
            case SIZE_SOUTH:
            {
                AreaQuad.Bottom += OffsetY;
            } break;
            
            case SIZE_WN:
            {
                AreaQuad.Left += OffsetX;
                AreaQuad.Top  += OffsetY;
            } break;
            
            case SIZE_ES:
            {
                AreaQuad.Right  += OffsetX;
                AreaQuad.Bottom += OffsetY;
            } break;
            
            case SIZE_NE:
            {
                AreaQuad.Right += OffsetX;
                AreaQuad.Top   += OffsetY;
            } break;
            
            case SIZE_WS:
            {
                AreaQuad.Left   += OffsetX;
                AreaQuad.Bottom += OffsetY;
            } break;
        }
        
        if(Index == 0)
        {
            game_rect GridArea = ConvertMathRectToGameRect(AreaQuad);
            
            LevelEditor->EditorObject[Index].AreaQuad = GridArea;
            LevelEntity->GridEntity->GridArea = GridArea;
            
            LevelEntity->Configuration.GridBlockSize = CalculateGridBlockSize(RowAmount, ColAmount, GridArea.w, GridArea.h);
            
            LevelEditorUpdateCoordinates(Buffer, LevelEditor->Font, &LevelEditor->PosPanel, LevelEntity->GridEntity->GridArea, 
                                         Memory->PadRect, {0, 0, Buffer->Width, Buffer->Height});
            
        }
        else if(Index == 1)
        {
            LevelEditor->EditorObject[Index].AreaQuad = ConvertMathRectToGameRect(AreaQuad);
            
            LevelEntity->FigureEntity->FigureArea = LevelEditor->EditorObject[Index].AreaQuad;
            
        }
        else if(Index == 2)
        {
            
        }
        
    }
    
    if(LevelEditor->ObjectIsSelected)
    {
        s32 Index = LevelEditor->EditorObjectIndex;
        DEBUGRenderQuad(Buffer, &LevelEditor->EditorObject[Index].AreaQuad, {255, 255, 0}, 255);
    }
    
    if(LevelEditor->ResPanel.ResNumberSelected)
    {
        game_rect CurrentNumberTextureQuad = {};
        
        switch(LevelEditor->ResPanel.ResolutionType)
        {
            case TARGET_WIDTH:
            {
                CurrentNumberTextureQuad = LevelEditor->ResPanel.TargetWidthNumberButton.TextureQuad;
            } break;
            
            case TARGET_HEIGHT:
            {
                CurrentNumberTextureQuad = LevelEditor->ResPanel.TargetHeightNumberButton.TextureQuad;
            } break;
            
            case REF_WIDTH:
            {
                CurrentNumberTextureQuad = LevelEditor->ResPanel.ReferenceWidthButton.TextureQuad;
            } break;
            
            case REF_HEIGHT:
            {
                CurrentNumberTextureQuad = LevelEditor->ResPanel.ReferenceHeightButton.TextureQuad;
            } break;
        }
        
        DEBUGRenderQuadFill(Buffer, &CurrentNumberTextureQuad, {255, 0, 255}, 255);
    }
    
    RenderButtonQuad(Buffer, &LevelEditor->SwitchConfiguration, 255, 192, 203, 100);
    
    /* Position panel rendering */
    
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.HeaderButton,  128, 128, 128, 100);
    
    /* Switch buttons rendering */
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.LeftArrowButton,   0, 255, 0, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.SwitchNameButton,  0, 255, 0, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.RightArrowButton,  0, 255, 0, 100);
    
    /* First coordinate buttons rendering */
    
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.FirstNumberNameButton, 0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.FirstNumberButton, 0, 0, 255, 100);
    
    /* Second coordinate buttons rendering */
    
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.SecondNumberNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.SecondNumberButton, 0, 0, 255, 100);
    
    /*Third coordinate buttons rendering */
    
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.ThirdNumberNameButton, 0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.ThirdNumberButton, 0, 0, 255, 100);
    
    /*Fourth coordinate buttons rendering*/
    
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.FourthNumberNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->PosPanel.FourthNumberButton,  0, 0, 255, 100);
    
    /* Resolution panel rendering */
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ScalerHeaderButton,  128, 128, 128, 100);
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.LeftArrowButton,  0, 255, 0, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.SwitchButton,     0, 255, 0, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.RightArrowButton, 0, 255, 0, 100);
    
    /* Target resolution buttons*/
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.TargetResolutionHeaderButton,  128, 128, 128, 100);
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.TargetWidthNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.TargetWidthNumberButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.TargetHeightNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.TargetHeightNumberButton,  0, 0, 255, 100);
    
    /* Reference resolution buttons */
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ReferenceResolutionHeaderButton,  128, 128, 128, 100);
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ReferenceWidthNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ReferenceWidthButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ReferenceHeightNameButton,  0, 0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ReferenceHeightButton,  0, 0, 255, 100);
    
    RenderButtonQuad(Buffer, &LevelEditor->ResPanel.ApplyButton,  0, 255, 255, 100);
    
    
    if(LevelEditor->ButtonSelected)
    {
        DEBUGRenderQuadFill(Buffer, &LevelEditor->HighlightButtonQuad, {255, 255, 0}, 150);
    }
    
}

static void
LevelConfigUpdateAndRender(level_editor *LevelEditor, level_entity *LevelEntity, 
                           game_memory *Memory, game_offscreen_buffer *Buffer, game_input *Input)
{
    game_rect GridArea   = LevelEntity->GridEntity->GridArea;
    game_rect FigureArea = LevelEntity->FigureEntity->FigureArea;
    
    s32 FigureAmount   = LevelEntity->FigureEntity->FigureAmount;
    s32 RowAmount      = LevelEntity->GridEntity->RowAmount;
    s32 ColAmount      = LevelEntity->GridEntity->ColumnAmount;
    
    s32 NewFigureIndex = LevelEditor->CurrentFigureIndex;
    
    s32 CurrentLevelIndex = Memory->CurrentLevelIndex;
    if(LevelEditor->CurrentLevelIndex != CurrentLevelIndex)
    {
        LevelEditor->CurrentLevelIndex = CurrentLevelIndex;
        LevelEditorChangeGridCounters(LevelEditor, RowAmount, ColAmount, Buffer);
        LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, 
                                    LevelEditor, LevelEntity->LevelNumber, 
                                    CurrentLevelIndex, Buffer);
        
    }
    
    if(LevelEditor->LevelNumberSelected)
    {
        s32 DigitIndex = LevelEditor->LevelNumberBufferIndex;
        char NextDigit = GetNumberFromInput(DigitIndex, Input);
        
        if(NextDigit)
        {
            if(DigitIndex <= 2 && NextDigit != '\n')
            {
                LevelEditor->LevelNumberBuffer[DigitIndex] = NextDigit;
                LevelEditor->LevelNumberBufferIndex = ++DigitIndex;
                
                GameMakeTextureFromString(LevelEntity->LevelNumberTexture, 
                                          LevelEditor->LevelNumberBuffer, 
                                          &LevelEntity->LevelNumberQuad, 
                                          Memory->LevelNumberFont, 
                                          {255, 255, 255}, 
                                          Buffer);
                
                GameMakeTextureFromString(LevelEntity->LevelNumberShadowTexture, 
                                          LevelEditor->LevelNumberBuffer, 
                                          &LevelEntity->LevelNumberShadowQuad, 
                                          Memory->LevelNumberFont, 
                                          {0, 0, 0}, 
                                          Buffer);
                
                LevelEntity->LevelNumberQuad.x = (Buffer->Width - LevelEntity->LevelNumberQuad.w) - (LevelEntity->LevelNumberQuad.w / 2);
                LevelEntity->LevelNumberQuad.y = 0;
                
                LevelEntity->LevelNumberShadowQuad.x = LevelEntity->LevelNumberQuad.x + 3;
                LevelEntity->LevelNumberShadowQuad.y = LevelEntity->LevelNumberQuad.y + 3;
            }
        }
        
    }
    
    if(Input->Keyboard.Up.EndedDown)
    {
        NewFigureIndex -= 1;
    }
    else if(Input->Keyboard.Down.EndedDown)
    {
        NewFigureIndex += 1;
    }
    else if(Input->Keyboard.Left.EndedDown)
    {
        NewFigureIndex -= 2;
    }
    else if(Input->Keyboard.Right.EndedDown)
    {
        NewFigureIndex += 2;
    }
    else if(Input->Keyboard.Q_Button.EndedDown)
    {
        s32 PrevLevelNumber = CurrentLevelIndex - 1;
        if(PrevLevelNumber >= 0)
        {
            Memory->CurrentLevelIndex = PrevLevelNumber;
            
            LevelEntityUpdateLevelEntityFromMemory(Memory, Memory->CurrentLevelIndex, true, Buffer);
            
            LevelEditorChangeGridCounters(LevelEditor, LevelEntity->GridEntity->RowAmount,
                                          LevelEntity->GridEntity->ColumnAmount,
                                          Buffer);
            
            LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, 
                                        LevelEditor, LevelEntity->LevelNumber, 
                                        Memory->CurrentLevelIndex, Buffer);
            
            LevelEditorChangeFigureCounter(LevelEditor, LevelEntity->FigureEntity->FigureAmount, Buffer);
        }
    }
    else if(Input->Keyboard.E_Button.EndedDown)
    {
        s32 NextLevelNumber = CurrentLevelIndex + 1;
        if(NextLevelNumber < Memory->LevelMemoryAmount)
        {
            Memory->CurrentLevelIndex = NextLevelNumber;
            
            LevelEntityUpdateLevelEntityFromMemory(Memory, Memory->CurrentLevelIndex, true, Buffer);
            LevelEditorChangeGridCounters(LevelEditor, LevelEntity->GridEntity->RowAmount,
                                          LevelEntity->GridEntity->ColumnAmount,
                                          Buffer);
            
            LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, 
                                        LevelEditor, LevelEntity->LevelNumber, 
                                        Memory->CurrentLevelIndex, Buffer);
            
            LevelEditorChangeFigureCounter(LevelEditor, LevelEntity->FigureEntity->FigureAmount, Buffer);
        }
    }
    else if(Input->Keyboard.Enter.EndedDown)
    {
        if(LevelEditor->LevelNumberSelected)
        {
            LevelEntity->LevelNumber = strtol(LevelEditor->LevelNumberBuffer, 0, 10);
            LevelEditor->LevelNumberSelected = false;
            
            menu_entity* MenuEntity = 
                (menu_entity*) (((char*)Memory->LocalMemoryStorage) + (sizeof(level_entity)));
            
            MenuChangeButtonText(Memory->LevelNumberFont, LevelEditor->LevelNumberBuffer, MenuEntity, 
                                 &MenuEntity->Buttons[Memory->CurrentLevelIndex], {255, 255, 255}, Buffer);
        }
    }
    
    if(Input->MouseButtons[0].EndedDown)
    {
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->SwitchConfiguration.Quad))
        {
            if(LevelEditor->EditorType == LEVEL_EDITOR)
            {
                LevelEditor->EditorType = GAME_EDITOR;
                LevelEditorUpdateTextOnButton(Buffer, LevelEditor->Font, "Game configuration", LevelEditor->SwitchConfiguration.Texture, &LevelEditor->SwitchConfiguration.TextureQuad, &LevelEditor->SwitchConfiguration.Quad, {255, 255, 255});
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->SwitchConfiguration.Quad;
            }
            
        }
        
        if (IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEntity->LevelNumberQuad))
        {
            LevelEditor->LevelNumberSelected = true;
            LevelEditor->LevelNumberBufferIndex = 0;
            LevelEditor->OldLevelNumber = LevelEntity->LevelNumber;
            
            LevelEditor->LevelNumberBuffer[0] = '\0';
            LevelEditor->LevelNumberBuffer[1] = '\0';
            LevelEditor->LevelNumberBuffer[2] = '\0';
            LevelEditor->LevelNumberBuffer[3] = '\0';
        }
        else
        {
            if(LevelEditor->LevelNumberSelected)
            {
                LevelEditor->LevelNumberSelected = false;
                
                char StringBuffer[4] = {};
                sprintf(StringBuffer, "%d", LevelEntity->LevelNumber);
                
                GameMakeTextureFromString(LevelEntity->LevelNumberTexture, 
                                          StringBuffer, 
                                          &LevelEntity->LevelNumberQuad, 
                                          Memory->LevelNumberFont, 
                                          {255, 255, 255}, 
                                          Buffer);
                
                GameMakeTextureFromString(LevelEntity->LevelNumberShadowTexture, 
                                          StringBuffer, 
                                          &LevelEntity->LevelNumberShadowQuad, 
                                          Memory->LevelNumberFont, 
                                          {0, 0, 0}, 
                                          Buffer);
                
                
                LevelEntity->LevelNumberQuad.x = (Buffer->Width - LevelEntity->LevelNumberQuad.w) - (LevelEntity->LevelNumberQuad.w / 2);
                LevelEntity->LevelNumberQuad.y = 0;
                
                LevelEntity->LevelNumberShadowQuad.x = LevelEntity->LevelNumberQuad.x + 3;
                LevelEntity->LevelNumberShadowQuad.y = LevelEntity->LevelNumberQuad.y + 3;
            }
        }
        
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->LevelPropertiesQuad))
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->RowLabel.MinusQuad))
            {
                printf("minus row!\n");
                
                GridEntityNewGrid(Buffer, LevelEntity, RowAmount-1, ColAmount, LevelEditor);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->RowLabel.MinusQuad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->RowLabel.PlusQuad))
            {
                printf("plus row!\n");
                
                GridEntityNewGrid(Buffer, LevelEntity, RowAmount+1, ColAmount, LevelEditor);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->RowLabel.PlusQuad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ColumnLabel.MinusQuad))
            {
                printf("minus column!\n");
                
                GridEntityNewGrid(Buffer, LevelEntity, RowAmount, ColAmount-1, LevelEditor);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->ColumnLabel.MinusQuad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->ColumnLabel.PlusQuad))
            {
                printf("plus column!\n");
                
                GridEntityNewGrid(Buffer, LevelEntity, RowAmount, ColAmount+1, LevelEditor);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->ColumnLabel.PlusQuad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FigureLabel.MinusQuad))
            {
                printf("minus figure!\n");
                
                if(FigureAmount > 0)
                {
                    FigureUnitDeleteFigure(LevelEntity->FigureEntity, LevelEntity->FigureEntity->FigureAmount - 1);
                    FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                    GridEntityUpdateStickUnits(LevelEntity->GridEntity, LevelEntity->FigureEntity->FigureAmount);
                    
                    LevelEditorChangeFigureCounter(LevelEditor, FigureAmount - 1, Buffer);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->FigureLabel.MinusQuad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FigureLabel.PlusQuad))
            {
                printf("plus figure!\n");
                
                if(FigureAmount < LevelEntity->FigureEntity->FigureAmountReserved)
                {
                    FigureUnitAddNewFigure(LevelEntity->FigureEntity, O_figure, classic, 0.0f, Memory, Buffer);
                    FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                    GridEntityUpdateStickUnits(LevelEntity->GridEntity, LevelEntity->FigureEntity->FigureAmount);
                    
                    LevelEditorChangeFigureCounter(LevelEditor, FigureAmount + 1, Buffer);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->FigureLabel.PlusQuad;
            }
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FigurePropertiesQuad))
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FlipFigureButton.Quad))
            {
                printf("flip!\n");
                
                FigureUnitFlipHorizontally(&LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex]);
                FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->FlipFigureButton.Quad;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->RotateFigureButton.Quad))
            {
                printf("rotate!\n");
                
                FigureUnitRotateShellBy(&LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex], 90);
                LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Angle += 90.0f;
                FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->RotateFigureButton.Quad;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->TypeFigureButton.Quad))
            {
                printf("type!\n");
                
                if(LevelEditor->CurrentFigureIndex >= 0)
                {
                    FreeTexture(LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Texture);
                    
                    figure_type Type = LevelEditorGetNextFigureType(LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Type);
                    
                    figure_form Form = LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Form;
                    
                    FigureUnitInitFigure(&LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex], Form,Type, 0.0f, Memory, Buffer);
                    
                    FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->TypeFigureButton.Quad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->FormFigureButton.Quad))
            {
                printf("form!\n");
                
                if(LevelEditor->CurrentFigureIndex >= 0)
                {
                    FreeTexture(LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Texture);
                    
                    figure_form Form = LevelEditorGetNextFigureForm(LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Form);
                    
                    figure_type Type = LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex].Type;
                    
                    FigureUnitInitFigure(&LevelEntity->FigureEntity->FigureUnit[LevelEditor->CurrentFigureIndex], Form,Type, 0.0f,  Memory, Buffer);
                    
                    FigureEntityAlignHorizontally(LevelEntity->FigureEntity, LevelEntity->Configuration.InActiveBlockSize);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->FormFigureButton.Quad;
            }
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->IOPropertiesQuad))
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->SaveLevelButton.Quad))
            {
                printf("save!\n");
                
                SaveLevelToMemory(Memory, LevelEntity, Memory->CurrentLevelIndex);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->SaveLevelButton.Quad;
            }
            
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->LoadLevelButton.Quad))
            {
                printf("load!\n");
                
                LevelEntityUpdateLevelEntityFromMemory(Memory, Memory->CurrentLevelIndex,
                                                       false, Buffer);
                LevelEditorChangeGridCounters(LevelEditor, 
                                              LevelEntity->GridEntity->RowAmount, LevelEntity->GridEntity->ColumnAmount, 
                                              Buffer);
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->LoadLevelButton.Quad;
            }
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &FigureArea))
        {
            for(u32 i = 0; i < FigureAmount; ++i)
            {
                game_rect AreaQuad = FigureUnitGetArea(&LevelEntity->FigureEntity->FigureUnit[i]);
                if(IsPointInsideRect(Input->MouseX, Input->MouseY, &AreaQuad))
                {
                    NewFigureIndex = i;
                }
            }
            
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->PrevLevelQuad))
            {
                s32 PrevLevelIndex = Memory->CurrentLevelIndex - 1;
                if(PrevLevelIndex >= 0)
                {
                    Memory->CurrentLevelIndex = PrevLevelIndex;
                    
                    LevelEntityUpdateLevelEntityFromMemory(Memory, PrevLevelIndex, true, Buffer);
                    
                    LevelEditorChangeGridCounters(LevelEditor, LevelEntity->GridEntity->RowAmount,
                                                  LevelEntity->GridEntity->ColumnAmount,Buffer);
                    
                    LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, LevelEditor, LevelEntity->LevelNumber, 
                                                Memory->CurrentLevelIndex, Buffer);
                    
                    LevelEditorChangeFigureCounter(LevelEditor, LevelEntity->FigureEntity->FigureAmount, Buffer);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->PrevLevelQuad;
            }
            else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &LevelEditor->NextLevelQuad))
            {
                s32 NextLevelIndex = Memory->CurrentLevelIndex + 1;
                if(NextLevelIndex < Memory->LevelMemoryAmount)
                {
                    Memory->CurrentLevelIndex = NextLevelIndex;
                    
                    LevelEntityUpdateLevelEntityFromMemory(Memory, NextLevelIndex, true, Buffer);
                    
                    LevelEditorChangeGridCounters(LevelEditor, 
                                                  LevelEntity->GridEntity->RowAmount, LevelEntity->GridEntity->ColumnAmount, Buffer);
                    
                    LevelEditorUpdateLevelStats(LevelEditor->LevelPropertiesQuad.w + 10, 0, LevelEditor, LevelEntity->LevelNumber, 
                                                Memory->CurrentLevelIndex, Buffer);
                    
                    
                    LevelEditorChangeFigureCounter(LevelEditor, LevelEntity->FigureEntity->FigureAmount, Buffer);
                }
                
                LevelEditor->ButtonSelected = true;
                LevelEditor->HighlightButtonQuad = LevelEditor->NextLevelQuad;
            }
            
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &GridArea))
        {
            if(!LevelEditor->ObjectIsSelected)
            {
                game_rect AreaQuad = { 0, 0, (s32)LevelEntity->Configuration.ActiveBlockSize, (s32)LevelEntity->Configuration.ActiveBlockSize };
                
                u32 StartX = 0;
                u32 StartY = 0;
                
                for(u32 Row = 0; Row < RowAmount; ++Row)
                {
                    StartY = GridArea.y + (LevelEntity->Configuration.ActiveBlockSize * Row);
                    
                    for(u32 Col = 0; Col < ColAmount; ++Col)
                    {
                        StartX = GridArea.x + (LevelEntity->Configuration.ActiveBlockSize * Col);
                        
                        AreaQuad.x = StartX;
                        AreaQuad.y = StartY;
                        
                        if(IsPointInsideRect(Input->MouseX, Input->MouseY, 
                                             &AreaQuad))
                        {
                            s32 UnitIndex = (Row * ColAmount) + Col;
                            u32 GridUnit = LevelEntity->GridEntity->UnitField[UnitIndex];
                            if(GridUnit == 0)
                            {
                                LevelEntity->GridEntity->UnitField[UnitIndex] = 1;
                            }
                            else
                            {
                                s32 Index = -1;
                                for(u32 m = 0; m < LevelEntity->GridEntity->MovingBlocksAmount; ++m)
                                {
                                    u32 RowNumber = LevelEntity->GridEntity->MovingBlocks[m].RowNumber;
                                    u32 ColNumber = LevelEntity->GridEntity->MovingBlocks[m].ColNumber;
                                    
                                    if((Row == RowNumber) && (Col == ColNumber))
                                    {
                                        Index = m;
                                        break;
                                    }
                                }
                                
                                if(Index >= 0)
                                {
                                    
                                    if(!LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch)
                                    {
                                        LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch = true;
                                    }
                                    else if(!LevelEntity->GridEntity->MovingBlocks[Index].IsVertical)
                                    {
                                        LevelEntity->GridEntity->MovingBlocks[Index].IsVertical = true;
                                        LevelEntity->GridEntity->MovingBlocks[Index].MoveSwitch = false;
                                    }
                                    else
                                    {
                                        GridEntityDeleteMovingBlock(LevelEntity->GridEntity, Index);
                                        LevelEntity->GridEntity->UnitField[UnitIndex] = 0;
                                    }
                                }
                                else
                                {
                                    GridEntityAddMovingBlock(LevelEntity->GridEntity, Row, Col, false, false, LevelEntity->Configuration.ActiveBlockSize);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if(Input->MouseButtons[0].EndedUp)
    {
        LevelEditor->ButtonSelected      = true;
        LevelEditor->HighlightButtonQuad = {};
    }
    
    if(NewFigureIndex < 0)
    {
        NewFigureIndex = FigureAmount - 1;
    }
    
    if(NewFigureIndex > FigureAmount - 1)
    {
        NewFigureIndex = 0;
    }
    
    LevelEditor->CurrentFigureIndex = NewFigureIndex;
    
    if(LevelEntity->LevelStarted)
    {
        //printf("FigureAmount = %d\n", FigureAmount);
        for(u32 i = 0; i < FigureAmount; ++i)
        {
            DEBUGRenderFigureShell(Buffer, &LevelEntity->FigureEntity->FigureUnit[i], LevelEntity->Configuration.InActiveBlockSize / 4, {0, 0, 255}, 255);
            DEBUGRenderQuad(Buffer, &LevelEntity->FigureEntity->FigureUnit[i].AreaQuad, {255, 0, 0}, 255);
        }
        
        if(LevelEntity->FigureEntity->FigureAmount > 0)
        {
            DEBUGRenderFigureShell(Buffer, &LevelEntity->FigureEntity->FigureUnit[NewFigureIndex],LevelEntity->Configuration.InActiveBlockSize, {255,
                                   255, 255}, 100);
        }
    }
    
    
    /* Level number is selected */ 
    
    if(LevelEditor->LevelNumberSelected)
    {
        DEBUGRenderQuadFill(Buffer, &LevelEntity->LevelNumberQuad, {255, 0, 0}, 150);
    }
    
    RenderButtonQuad(Buffer, &LevelEditor->SwitchConfiguration, 255, 192, 203, 100);
    
    /* Level header name rendering */ 
    
    //RenderButtonQuad(Buffer, &LevelEditor->LevelConfigButton, 128, 128, 128, 100);
    
    LevelEditorRenderLabel(&LevelEditor->RowLabel,    Buffer);
    LevelEditorRenderLabel(&LevelEditor->ColumnLabel, Buffer);
    LevelEditorRenderLabel(&LevelEditor->FigureLabel, Buffer);
    
    /* Figure header name rendering */ 
    
    RenderButtonQuad(Buffer, &LevelEditor->FigureConfigButton, 128, 128, 128, 100);
    RenderButtonQuad(Buffer, &LevelEditor->FlipFigureButton,     0,   0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->RotateFigureButton,   0,   0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->TypeFigureButton,     0,   0, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->FormFigureButton,     0,   0, 255, 100);
    
    /* IO header name rendering */ 
    
    RenderButtonQuad(Buffer, &LevelEditor->IOConfigButton,  128, 128, 128, 100);
    RenderButtonQuad(Buffer, &LevelEditor->SaveLevelButton,   0, 255, 255, 100);
    RenderButtonQuad(Buffer, &LevelEditor->LoadLevelButton,   0, 255, 255, 100);
    
    /* Left/Right arrows rendering */ 
    
    
    if(CurrentLevelIndex > 0)
    {
        GameRenderBitmapToBuffer(Buffer, LevelEditor->PrevLevelTexture, &LevelEditor->PrevLevelQuad);
    }
    
    if(CurrentLevelIndex < (s32)Memory->LevelMemoryAmount-1)
    {
        GameRenderBitmapToBuffer(Buffer, LevelEditor->NextLevelTexture, &LevelEditor->NextLevelQuad);
    }
    
    /* Prev/Next level buttons rendering */
    
    GameRenderBitmapToBuffer(Buffer, LevelEditor->LevelIndexTexture, &LevelEditor->LevelIndexQuad);
    
    DEBUGRenderQuad(Buffer, &LevelEntity->GridEntity->GridArea, { 0, 255, 255 }, 255);
    
    
    if(LevelEditor->ButtonSelected)
    {
        DEBUGRenderQuadFill(Buffer, &LevelEditor->HighlightButtonQuad, {255, 255, 0}, 150);
    }
}

static void
LevelEditorUpdateAndRender(level_editor *LevelEditor, level_entity *LevelEntity, 
                           game_memory *Memory, game_offscreen_buffer *Buffer, game_input *Input)
{
    if(Input->Keyboard.BackQuote.EndedDown)
    {
        if(!LevelEditor->IsOn)
        {
            LevelEditor->IsOn  = true;
            LevelEntity->LevelPaused = true;
            RestartLevelEntity(LevelEntity);
        }
        else
        {
            LevelEditor->IsOn  = false;
            LevelEntity->LevelPaused = false;
        }
    }
    
    if(!LevelEditor->IsOn) 
    {
        return;
    }
    
    if(LevelEditor->EditorType == LEVEL_EDITOR)
    {
        LevelConfigUpdateAndRender(LevelEditor, LevelEntity, 
                                   Memory, Buffer, Input);
    }
    else if(LevelEditor->EditorType == GAME_EDITOR)
    {
        GameConfigUpdateAndRender(LevelEditor, LevelEntity,
                                  Memory, Buffer, Input);
    }
    
}

struct selection_panel
{
    bool IsSelected;
    game_rect SelectQuad;
};

struct menu_editor
{
    /* For showing gui */
    bool EditorMode;
    bool ButtonsSelected;
    
    /* Button data for adding a new level in memory */
    game_rect NewLevelButtonQuad;
    game_rect NewLevelTextureQuad;
    game_texture *NewLevelTexture;
    
    /* Button data for loading levels from memory*/
    game_rect LoadButtonQuad;
    game_rect LoadButtonTextureQuad;
    game_texture *LoadButtonTexture;
    
    /* Button data for saving levels on the disk */
    game_rect SaveButtonQuad;
    game_rect SaveButtonTextureQuad;
    game_texture *SaveButtonTexture;
    
    /* Button data for sorting levels in memory by their level number*/
    game_rect SortButtonQuad;
    game_rect SortButtonTextureQuad;
    game_texture *SortButtonTexture;
    
    /* Button data for confirming deletion of a level */
    game_rect DeleteButtonQuad;
    game_rect DeleteButtonTextureQuad;
    game_texture *DeleteButtonTexture;
    
    /* Button data for canceling deletion of a level */
    game_rect CancelButtonQuad;
    game_rect CancelButtonTextureQuad;
    game_texture *CancelButtonTexture;
    
    /* For highlightning the button whenever pressed */
    bool ButtonIsPressed;
    game_rect HighlightButtonQuad;
    
    /* For highlightning selected level buttons  */
    selection_panel SelectionPanel[100];
};

static void
MenuEditorInit(menu_editor *MenuEditor, menu_entity *MenuEntity, 
               game_memory *Memory, game_offscreen_buffer *Buffer)
{
    *MenuEditor = {};
    
    s32 ButtonWidth  = 150;
    s32 ButtonHeigth = 50;
    
    MenuMakeTextButton("New", 0, 0, ButtonWidth, ButtonHeigth,
                       &MenuEditor->NewLevelButtonQuad,
                       &MenuEditor->NewLevelTextureQuad,
                       MenuEditor->NewLevelTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
    
    MenuMakeTextButton("Load", 0, 50, ButtonWidth, ButtonHeigth,
                       &MenuEditor->LoadButtonQuad,
                       &MenuEditor->LoadButtonTextureQuad,
                       MenuEditor->LoadButtonTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
    
    MenuMakeTextButton("Save", 0, 100, ButtonWidth, ButtonHeigth,
                       &MenuEditor->SaveButtonQuad,
                       &MenuEditor->SaveButtonTextureQuad,
                       MenuEditor->SaveButtonTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
    
    MenuMakeTextButton("Sort", 0, 150, ButtonWidth, ButtonHeigth,
                       &MenuEditor->SortButtonQuad,
                       &MenuEditor->SortButtonTextureQuad,
                       MenuEditor->SortButtonTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
    
    MenuMakeTextButton("Delete", 0, 250, ButtonWidth, ButtonHeigth,
                       &MenuEditor->DeleteButtonQuad,
                       &MenuEditor->DeleteButtonTextureQuad,
                       MenuEditor->DeleteButtonTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
    
    MenuMakeTextButton("Cancel", 0, 300, ButtonWidth, ButtonHeigth,
                       &MenuEditor->CancelButtonQuad,
                       &MenuEditor->CancelButtonTextureQuad,
                       MenuEditor->CancelButtonTexture,
                       Memory->LevelNumberFont,
                       {255, 255, 255}, Buffer);
}

inline void
SwapLevelMemory(level_memory *LevelMemory, s32 IndexA, s32 IndexB)
{
    level_memory TempLevel = LevelMemory[IndexA];
    LevelMemory[IndexA] = LevelMemory[IndexB];
    LevelMemory[IndexB] = TempLevel;
}

static void
MenuEditorSortButtonsRange(s32 BeginIndex, s32 EndIndex, menu_entity *MenuEntity, game_memory *Memory)
{
    level_memory *LevelMemory = (level_memory*)Memory->GlobalMemoryStorage;
    
    s32 LevelAmount = Memory->LevelMemoryAmount;
    
    for(s32 i = BeginIndex; i < EndIndex - 1; ++i)
    {
        for(s32 j = BeginIndex; j < EndIndex - 1; ++j)
        {
            if(LevelMemory[j].LevelNumber > LevelMemory[j+1].LevelNumber)
            {
                SwapLevelMemory(LevelMemory, j, j+1);
            }
        }
    }
}

static void
MenuEditorUpdateAndRender(menu_editor *MenuEditor, menu_entity *MenuEntity, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    
    if(Input->Keyboard.BackQuote.EndedDown)
    {
        if(!MenuEditor->EditorMode)
        {
            MenuEditor->EditorMode = true;
            MenuEntity->IsPaused = true;
        }
        else
        {
            MenuEditor->EditorMode = false;
            MenuEntity->IsPaused = false;
        }
    }
    
    if(!MenuEditor->EditorMode)
    {
        return;
    }
    
    if(Input->MouseButtons[0].EndedDown)
    {
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->NewLevelButtonQuad))
        {
            if(MenuEntity->ButtonsAmount < MenuEntity->ButtonsAmountReserved)
            {
                MenuEntity->ButtonsAmount += 1;
                Memory->LevelMemoryAmount += 1;
                
                MenuEntityAlignButtons(MenuEntity, Buffer->Width, Buffer->Height);
            }
            
            MenuEditor->ButtonIsPressed = true;
            MenuEditor->HighlightButtonQuad = MenuEditor->NewLevelButtonQuad;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->LoadButtonQuad))
        {
            LoadLevelMemoryFromFile("package2.bin", Memory);
            MenuLoadButtonsFromMemory(MenuEntity, Memory, Buffer);
            
            if(MenuEditor->ButtonsSelected)
            {
                MenuEditor->ButtonsSelected = false;
                
                for(s32 Index = 0; Index < 100; Index++)
                {
                    if(MenuEditor->SelectionPanel[Index].IsSelected)
                    {
                        MenuEditor->SelectionPanel[Index].IsSelected = false;
                    }
                }
            }
            
            MenuEditor->ButtonIsPressed = true;
            MenuEditor->HighlightButtonQuad = MenuEditor->LoadButtonQuad;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->SaveButtonQuad))
        {
            SaveLevelMemoryToFile(Memory);
            
            MenuEditor->ButtonIsPressed = true;
            MenuEditor->HighlightButtonQuad = MenuEditor->SaveButtonQuad;
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->SortButtonQuad))
        {
            if(MenuEditor->ButtonsSelected) 
            {
                MenuEditor->ButtonsSelected = false;
            }
            
            bool BeginFound = false;
            
            s32 BeginIndex = 0;
            s32 EndIndex   = 99;
            
            for(s32 Index = 0; Index < 100; ++Index)
            {
                if(MenuEditor->SelectionPanel[Index].IsSelected)
                {
                    MenuEditor->SelectionPanel[Index].IsSelected = false;
                    
                    if(!BeginFound)
                    {
                        BeginFound = true;
                        BeginIndex = Index;
                        
                    }
                }
                else
                {
                    if(BeginFound)
                    {
                        EndIndex = Index;
                        break;
                    }
                }
            }
            
            if(EndIndex > MenuEntity->ButtonsAmount) 
            {
                EndIndex = MenuEntity->ButtonsAmount;
            }
            
            MenuEditorSortButtonsRange(BeginIndex, EndIndex, MenuEntity, Memory);
            MenuLoadButtonsFromMemory(MenuEntity, Memory, Buffer);
            
            MenuEditor->ButtonIsPressed = true;
            MenuEditor->HighlightButtonQuad = MenuEditor->SortButtonQuad;
        }
        
        if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->DeleteButtonQuad))
        {
            if(MenuEditor->ButtonsSelected)
            {
                s32 LevelCounter = 0;
                for(s32 Index = 100; Index > 0; Index--)
                {
                    if(MenuEditor->SelectionPanel[Index].IsSelected)
                    {
                        MenuEditor->SelectionPanel[Index].IsSelected = false;
                        MenuDeleteLevel(MenuEntity, Index, Memory, Buffer);
                        
                        ++LevelCounter;
                    }
                }
                
                Memory->CurrentLevelIndex -= LevelCounter;
                Memory->CurrentLevelIndex = (LevelCounter > Memory->CurrentLevelIndex)
                    ? Memory->CurrentLevelIndex = 0
                    : Memory->CurrentLevelIndex - LevelCounter;
            }
        }
        else if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEditor->CancelButtonQuad))
        {
            if(MenuEditor->ButtonsSelected)
            {
                MenuEditor->ButtonsSelected = false;
                
                for(s32 Index = 0; Index < 100; ++Index)
                {
                    if(MenuEditor->SelectionPanel[Index].IsSelected)
                    {
                        MenuEditor->SelectionPanel[Index].IsSelected = false;
                    }
                }
                
                MenuEditor->ButtonIsPressed = true;
                MenuEditor->HighlightButtonQuad = MenuEditor->CancelButtonQuad;
            }
        }
        
    }
    else if(Input->MouseButtons[0].EndedUp)
    {
        MenuEditor->ButtonIsPressed = false;
        MenuEditor->HighlightButtonQuad = {};
    }
    
    if(Input->MouseButtons[1].EndedDown)
    {
        s32 Index = -1;
        
        u32 BeginIndex = MenuEntity->TargetIndex * 20;
        u32 EndIndex   = BeginIndex + 20;
        
        for(u32 i = BeginIndex; i < EndIndex; ++i)
        {
            if(IsPointInsideRect(Input->MouseX, Input->MouseY, &MenuEntity->Buttons[i].ButtonQuad))
            {
                Index = i;
                break;
            }
        }
        
        if(Index >= 0)
        {
            MenuEditor->ButtonsSelected = true;
            
            if(!MenuEditor->SelectionPanel[Index].IsSelected)
            {
                MenuEditor->SelectionPanel[Index].IsSelected = true;
                MenuEditor->SelectionPanel[Index].SelectQuad = MenuEntity->Buttons[Index].ButtonQuad;
            }
            else
            {
                MenuEditor->SelectionPanel[Index].IsSelected = false;
            }
        }
    }
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->NewLevelButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->NewLevelButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->NewLevelTexture, &MenuEditor->NewLevelTextureQuad);
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->LoadButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->LoadButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->LoadButtonTexture, &MenuEditor->LoadButtonTextureQuad);
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->SaveButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->SaveButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->SaveButtonTexture, &MenuEditor->SaveButtonTextureQuad);
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->SortButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->SortButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->SortButtonTexture, &MenuEditor->SortButtonTextureQuad);
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->DeleteButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->DeleteButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->DeleteButtonTexture, &MenuEditor->DeleteButtonTextureQuad);
    
    DEBUGRenderQuadFill(Buffer, &MenuEditor->CancelButtonQuad, {128, 128, 128}, 100);
    DEBUGRenderQuad(Buffer, &MenuEditor->CancelButtonQuad, {255, 255, 255}, 255);
    GameRenderBitmapToBuffer(Buffer, MenuEditor->CancelButtonTexture, &MenuEditor->CancelButtonTextureQuad);
    
    if(MenuEditor->ButtonsSelected)
    {
        for(s32 i = 0; i < 100; ++i)
        {
            if(MenuEditor->SelectionPanel[i].IsSelected)
            {
                DEBUGRenderQuadFill(Buffer, &MenuEditor->SelectionPanel[i].SelectQuad, {0, 255, 255}, 100);
            }
        }
    }
    
    if(MenuEditor->ButtonIsPressed)
    {
        DEBUGRenderQuadFill(Buffer, &MenuEditor->HighlightButtonQuad, {0, 255, 255}, 200);
    }
    
    
}

