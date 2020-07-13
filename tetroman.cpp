/* ========================================= */
//     $File: tetroman.cpp
//     $Date: October 10th 2017 10:32 am 
//     $Creator: Maksim Sokolov
//     $Revision: $
//     $Description: $
/* ========================================= */

#include "tetroman.h"

#include "tetroman_render_group.cpp"
#include "tetroman_playground.cpp"
#include "tetroman_asset.cpp"
#include "tetroman_menu.cpp"
#include "tetroman_editor.cpp"

static bool
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
    bool ShouldQuit = false;
    
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);    
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    
    if(!Memory->IsInitialized)
    {
        // TODO(msokolov): should get rid of this initialization
        /* NOTE(msokolov): game_memory initialization starts here */
        
        /* NOTE(msokolov): game_state initialization starts here */
        InitializeMemoryGroup(&GameState->MemoryGroup, Memory->PermanentStorageSize - sizeof(game_state), (u8*)Memory->PermanentStorage + sizeof(game_state));
        
        GameState->Font = TTF_OpenFont("..\\data\\Karmina-Bold.otf", 50);
        Assert(GameState->Font);
        
        GameState->EditorMode      = false;
        GameState->PlaygroundIndex = 0;
        GameState->CurrentMode     = game_mode::PLAYGROUND;
        
        playground_config *Configuration = &GameState->Configuration;
        Configuration->StartUpTimeToFinish = 0.0f;
        Configuration->RotationVel         = 600.0f;
        Configuration->StartAlphaPerSec    = 500.0f;
        Configuration->FlippingAlphaPerSec = 1000.0f;
        Configuration->FigureVelocity      = 600.0f;
        Configuration->MovingBlockVelocity = 555.0f;
        
        /* NOTE(msokolov): playground initialization starts here */
        playground* Playground = &GameState->Playground;
        Playground->LevelNumber           = 0;
        Playground->LevelStarted          = true;
        Playground->LevelFinished         = false;
        Playground->LevelPaused           = false;
        
        
        
        /* NOTE(msokolov): figure_entity initialization starts here */
        figure_entity* FigureEntity  = &Playground->FigureEntity;
        FigureEntity->FigureAmount   = 0;
        FigureEntity->ReturnIndex    = -1;
        FigureEntity->FigureActive   = -1;
        FigureEntity->IsGrabbed      = false;
        FigureEntity->IsRotating     = false;
        FigureEntity->IsReturning    = false;
        FigureEntity->IsRestarting   = false;
        FigureEntity->IsFlipping     = false;
        FigureEntity->RotationSum    = 0;
        FigureEntity->AreaAlpha      = 0;
        FigureEntity->FigureAlpha    = 0;
        FigureEntity->FadeInSum      = 0;
        FigureEntity->FadeOutSum     = 0;
        FigureEntity->FigureVelocity = 400.0f;
        FigureEntity->RotationVelocity = Configuration->RotationVel;
        
        FigureEntity->FigureArea.Min.x = 1500;
        FigureEntity->FigureArea.Min.y = 81;
        FigureEntity->FigureArea.Max.x = FigureEntity->FigureArea.Min.x + 552;
        FigureEntity->FigureArea.Max.y = FigureEntity->FigureArea.Min.y + 972;
        
        FigureUnitAddNewFigure(FigureEntity, L_figure, classic);
        FigureUnitAddNewFigure(FigureEntity, O_figure, stone);
        FigureUnitAddNewFigure(FigureEntity, O_figure, mirror);
        
        FigureEntityAlignFigures(&Playground->FigureEntity);
        
        for(u32 i = 0; i < FIGURE_AMOUNT_MAXIMUM; ++i) 
        {
            FigureEntity->FigureOrder[i] = i;
        }
        
        FigureEntity->O_ClassicTexture = GetTexture(Memory, "o_d.png", Buffer->Renderer);
        FigureEntity->O_StoneTexture   = GetTexture(Memory, "o_s.png", Buffer->Renderer);
        FigureEntity->O_MirrorTexture  = GetTexture(Memory, "o_m.png", Buffer->Renderer);
        
        FigureEntity->I_ClassicTexture = GetTexture(Memory, "i_d.png", Buffer->Renderer);
        FigureEntity->I_StoneTexture   = GetTexture(Memory, "i_s.png", Buffer->Renderer);
        FigureEntity->I_MirrorTexture  = GetTexture(Memory, "i_m.png", Buffer->Renderer);
        
        FigureEntity->L_ClassicTexture = GetTexture(Memory, "l_d.png", Buffer->Renderer);
        FigureEntity->L_StoneTexture   = GetTexture(Memory, "l_s.png", Buffer->Renderer);
        FigureEntity->L_MirrorTexture  = GetTexture(Memory, "l_m.png", Buffer->Renderer);
        
        FigureEntity->J_ClassicTexture = GetTexture(Memory, "j_d.png", Buffer->Renderer);
        FigureEntity->J_StoneTexture   = GetTexture(Memory, "j_s.png", Buffer->Renderer);
        FigureEntity->J_MirrorTexture  = GetTexture(Memory, "j_m.png", Buffer->Renderer);
        
        FigureEntity->Z_ClassicTexture = GetTexture(Memory, "z_d.png", Buffer->Renderer);
        FigureEntity->Z_StoneTexture   = GetTexture(Memory, "z_s.png", Buffer->Renderer);
        FigureEntity->Z_MirrorTexture  = GetTexture(Memory, "z_m.png", Buffer->Renderer);
        
        FigureEntity->S_ClassicTexture = GetTexture(Memory, "s_d.png", Buffer->Renderer);
        FigureEntity->S_StoneTexture   = GetTexture(Memory, "s_s.png", Buffer->Renderer);
        FigureEntity->S_MirrorTexture  = GetTexture(Memory, "s_m.png", Buffer->Renderer);
        
        FigureEntity->T_ClassicTexture = GetTexture(Memory, "t_d.png", Buffer->Renderer);
        FigureEntity->T_StoneTexture   = GetTexture(Memory, "t_s.png", Buffer->Renderer);
        FigureEntity->T_MirrorTexture  = GetTexture(Memory, "t_m.png", Buffer->Renderer);
        
        /* NOTE(msokolov): grid_entity initialization starts here */
        grid_entity *GridEntity = &Playground->GridEntity;
        GridEntity->RowAmount           = 8;
        GridEntity->ColumnAmount        = 6;
        GridEntity->StickUnitsAmount    = FigureEntity->FigureAmount;
        GridEntity->MovingBlocksAmount  = 1;
        GridEntity->MovingBlockVelocity = Configuration->MovingBlockVelocity;
        
        GridEntity->GridArea.Min.x = 100;
        GridEntity->GridArea.Min.y = 81;
        GridEntity->GridArea.Max.x = GridEntity->GridArea.Min.x + 1128;
        GridEntity->GridArea.Max.y = GridEntity->GridArea.Min.y + 972;
        
        for (u32 Row = 0; Row < ROW_AMOUNT_MAXIMUM; ++Row)
        {
            for (u32 Col = 0; Col < COLUMN_AMOUNT_MAXIMUM; ++Col)
            {
                GridEntity->UnitField[(Row * COLUMN_AMOUNT_MAXIMUM) + Col] = 0;
            }
        }
        
        for (u32 i = 0; i < FIGURE_AMOUNT_MAXIMUM; ++i)
        {
            GridEntity->StickUnits[i].Index     = -1;
            GridEntity->StickUnits[i].IsSticked = false;
        }
        
        GridEntity->NormalSquareTexture     = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
        GridEntity->VerticalSquareTexture   = GetTexture(Memory, "o_s.png", Buffer->Renderer);
        GridEntity->HorizontlaSquareTexture = GetTexture(Memory, "o_m.png", Buffer->Renderer);
        GridEntity->TopLeftCornerFrame      = GetTexture(Memory, "frame3.png", Buffer->Renderer);
        GridEntity->TopRightCornerFrame     = GetTexture(Memory, "frame4.png", Buffer->Renderer);
        GridEntity->DownLeftCornerFrame     = GetTexture(Memory, "frame2.png", Buffer->Renderer);
        GridEntity->DownRightCornerFrame    = GetTexture(Memory, "frame1.png", Buffer->Renderer);
        
        for (u32 BlockIndex = 0; 
             BlockIndex < GridEntity->MovingBlocksAmount;
             ++BlockIndex)
        {
            moving_block *Block = &GridEntity->MovingBlocks[BlockIndex];
            
            Block->Area.Min.x = GridEntity->GridArea.Min.x + (Block->ColNumber * GRID_BLOCK_SIZE);
            Block->Area.Min.y = GridEntity->GridArea.Min.y + (Block->RowNumber * GRID_BLOCK_SIZE);
            Block->Area.Max.x = Block->Area.Min.x + GRID_BLOCK_SIZE;
            Block->Area.Max.y = Block->Area.Min.y + GRID_BLOCK_SIZE;
            
            GridEntity->UnitField[(Block->RowNumber * COLUMN_AMOUNT_MAXIMUM) + Block->ColNumber] = 2;
        }
        
        /* NOTE(msokolov): menu_entity initialization starts here */ 
#if 0
        GameState->MenuEntity   = PushStruct(&GameState->MemoryGroup, menu_entity);
        menu_entity *MenuEntity = GameState->MenuEntity;
        MenuEntity->MaxVelocity = 20.0f;
        MenuEntity->ButtonIndex = -1;
        MenuEntity->BackTexture = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
#endif
        
        Memory->IsInitialized = true;
        printf("Memory has been initialized!\n");
        
        /* NOTE(msokolov): just for testing */
        /* playground data from binary initialization */
        playground_data *PlaygroundData = (playground_data *) Memory->LevelStorage;
        Assert(PlaygroundData);
        
        GameState->PlaygroundData = PlaygroundData;
        PlaygroundData[0].FigureAmount = 3;
        PlaygroundData[0].Figures[0].Form = figure_form::I_figure;
        PlaygroundData[0].Figures[0].Type = figure_type::classic;
        
        PlaygroundData[0].Figures[1].Form = figure_form::O_figure;
        PlaygroundData[0].Figures[1].Type = figure_type::classic;
        
        PlaygroundData[0].Figures[2].Form = figure_form::L_figure;
        PlaygroundData[0].Figures[2].Type = figure_type::classic;
        
        PlaygroundData[0].RowAmount    = 5;
        PlaygroundData[0].ColumnAmount = 5;
        PlaygroundData[0].MovingBlocksAmount = 0;
        PlaygroundData[0].MovingBlocks[0].RowNumber = 1;
        PlaygroundData[0].MovingBlocks[0].ColNumber = 1;
        
        PrepareNextPlayground(Playground, Configuration, PlaygroundData, 0);
        
        char LevelString[3] = {};
        sprintf(LevelString, "%d", Playground->LevelNumber);
        if (Playground->LevelNumberTexture)
        {
            SDL_DestroyTexture(Playground->LevelNumberTexture);
        }
        
        Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
        Assert(Playground->LevelNumberTexture);
        //WriteLevelsToFile(Memory->LevelStorage, Memory->LevelStorageSize);
        
        /* NOTE(msokolov): playground_menu initialization */
        GameState->PlaygroundMenu.LevelButtonTexture = GetTexture(Memory, "grid_cell.png", Buffer->Renderer);
        Assert(GameState->PlaygroundMenu.LevelButtonTexture);
        
        for(u32 Index = 0;
            Index < 100;
            ++Index)
        {
            char LevelString[3] = {};
            sprintf(LevelString, "%d", Index + 1);
            
            if (GameState->PlaygroundMenu.LevelNumberTexture[Index])
            {
                SDL_DestroyTexture(GameState->PlaygroundMenu.LevelNumberTexture[Index]);
            }
            
            GameState->PlaygroundMenu.LevelNumberTexture[Index] = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
        }
        
#if DEBUG_BUILD
        
        playground_editor *PlaygroundEditor = PushStruct(&GameState->MemoryGroup, playground_editor);
        GameState->PlaygroundEditor = PlaygroundEditor;
        
        PlaygroundEditor->Font = TTF_OpenFont("..\\data\\Karmina-Bold.otf", 50);
        Assert(PlaygroundEditor->Font);
        
        PlaygroundEditor->SelectedArea = selected_area::GRID_PLAYGROUND;
        PlaygroundEditor->FigureIndex  = 0;
        
        PlaygroundEditor->FigureFormTexture   = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Shape", {255, 255, 255, 255});
        PlaygroundEditor->FigureRotateTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Rotate", {255, 255, 255, 255});
        PlaygroundEditor->FigureAddTexture    = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Add", {255, 255, 255, 255});
        PlaygroundEditor->FigureDeleteTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Delete", {255, 255, 255, 255});
        
        PlaygroundEditor->FigureButtonsArea.Min.x = FigureEntity->FigureArea.Min.x - 180.0f; 
        PlaygroundEditor->FigureButtonsArea.Min.y = FigureEntity->FigureArea.Min.y + (GetDim(FigureEntity->FigureArea).h / 2.0f) - (180.0f / 2.0f); 
        SetDim(&PlaygroundEditor->FigureButtonsArea, 180, 240);
        
        PlaygroundEditor->GridRowTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Row", {255, 255, 255, 255});;
        PlaygroundEditor->GridColumnTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Column", {255, 255, 255, 255});;
        PlaygroundEditor->GridPlusTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "+", {255, 255, 255, 255});;
        PlaygroundEditor->GridMinusTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "-", {255, 255, 255, 255});;
        
        PlaygroundEditor->GridButtonsArea.Min.x = FigureEntity->FigureArea.Min.x; 
        PlaygroundEditor->GridButtonsArea.Min.y = FigureEntity->FigureArea.Max.y - (GetDim(FigureEntity->FigureArea).h / 2.0f) - (60.0f);
        SetDim(&PlaygroundEditor->GridButtonsArea, 300.0f, 120.0f);
        
        //PlaygroundEditor->SavedTexture = MakeTextureFromString(Buffer, PlaygroundEditor->Font, "Saved.", {255, 255, 255, 255});
        
        PlaygroundEditor->IsInitialized = true;
#endif
        
    }
    
    Assert(sizeof(transient_state) < Memory->TransientStorageSize);
    transient_state *TransState = (transient_state *)Memory->TransientStorage;
    if (!TransState->IsInitialized)
    {
        InitializeMemoryGroup(&TransState->TransGroup, Memory->TransientStorageSize - sizeof(transient_state), (u8*)Memory->TransientStorage + sizeof(transient_state));
        
        TransState->IsInitialized = true;
    }
    
    playground *Playground   = &GameState->Playground;
    //menu_entity *MenuEntity  = GameState->MenuEntity;
    
    if(Input->Keyboard.Tab.EndedDown)
    {
        if(GameState->CurrentMode == PLAYGROUND) 
        {
            GameState->CurrentMode = LEVEL_MENU;
        }
        else if(GameState->CurrentMode == LEVEL_MENU)
        {
            GameState->CurrentMode = PLAYGROUND;
        }
    }
    
    if(Input->Keyboard.Escape.EndedDown)
    {
        ShouldQuit = true;
    }
    
    if(Input->Keyboard.BackQuote.EndedDown)
    {
        RestartLevelEntity(Playground);
        
#if DEBUG_BUILD
        if (!GameState->EditorMode)
        {
            GameState->EditorMode   = true;
            Playground->LevelPaused = true;
        }
        else
        {
            GameState->EditorMode   = false;
            Playground->LevelPaused = false;
        }
#endif
    }
    
#if DEBUG_BUILD
    
    if (GameState->EditorMode)
    {
        if(Input->Keyboard.E_Button.EndedDown)
        {
            u32 PlaygroundIndex = GameState->PlaygroundIndex;
            if (PlaygroundIndex < PLAYGROUND_MAXIMUM)
            {
                RestartLevelEntity(Playground);
                PlaygroundIndex++;
                
                playground_data *PlaygroundData = GameState->PlaygroundData;
                Assert(PlaygroundData);
                
                PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, PlaygroundIndex);
                
                char LevelString[3] = {};
                sprintf(LevelString, "%d", Playground->LevelNumber);
                if (Playground->LevelNumberTexture)
                {
                    SDL_DestroyTexture(Playground->LevelNumberTexture);
                }
                
                Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                Assert(Playground->LevelNumberTexture);
                
                printf("LevelIndex = %d\n", PlaygroundIndex);
            }
            
            GameState->PlaygroundEditor->FigureIndex = 0;
            GameState->PlaygroundIndex = PlaygroundIndex;
        }
        
        if (Input->Keyboard.Q_Button.EndedDown)
        {
            u32 PlaygroundIndex = GameState->PlaygroundIndex;
            if (PlaygroundIndex > 0)
            {
                RestartLevelEntity(Playground);
                
                PlaygroundIndex--;
                
                playground_data *PlaygroundData = GameState->PlaygroundData;
                Assert(PlaygroundData);
                
                PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, PlaygroundIndex);
                
                char LevelString[3] = {};
                sprintf(LevelString, "%d", Playground->LevelNumber);
                if (Playground->LevelNumberTexture)
                {
                    SDL_DestroyTexture(Playground->LevelNumberTexture);
                }
                
                Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                Assert(Playground->LevelNumberTexture);
                printf("LevelIndex = %d\n", PlaygroundIndex);
            }
            
            GameState->PlaygroundEditor->FigureIndex = 0;
            GameState->PlaygroundIndex = PlaygroundIndex;
        }
        
        if(Input->Keyboard.S_Button.EndedDown)
        {
            WritePlaygroundData(GameState->PlaygroundData, Playground, GameState->PlaygroundIndex);
            WriteLevelsToFile(Memory->LevelStorage, Memory->LevelStorageSize);
            printf("saved.\n");
        }
    }
    
#endif
    
    /*
 NOTE(msokolov): Render 
    */
    
    memory_group TemporaryMemory = TransState->TransGroup;
    render_group *RenderGroup = AllocateRenderGroup(&TransState->TransGroup, Kilobytes(100), Buffer->Width, Buffer->Height);
    
    game_mode CurrentMode = GameState->CurrentMode;
    switch (CurrentMode)
    {
        case PLAYGROUND:
        {
            playground_status PlaygroundStatus = PlaygroundUpdateAndRender(Playground, RenderGroup, Input);
            if (PlaygroundStatus == playground_status::LEVEL_FINISHED)
            {
                if (GameState->PlaygroundIndex < PLAYGROUND_MAXIMUM)
                {
                    GameState->PlaygroundIndex++;
                    
                    RestartLevelEntity(Playground);
                    playground_data *PlaygroundData = GameState->PlaygroundData;
                    Assert(PlaygroundData);
                    
                    PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, GameState->PlaygroundIndex);
                    
                    char LevelString[3] = {};
                    sprintf(LevelString, "%d", Playground->LevelNumber);
                    if (Playground->LevelNumberTexture)
                    {
                        SDL_DestroyTexture(Playground->LevelNumberTexture);
                    }
                    
                    Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                    Assert(Playground->LevelNumberTexture);
                }
            }
            
            if (GameState->EditorMode)
            {
                playground_editor *PlaygroundEditor = GameState->PlaygroundEditor;
                PlaygroundEditorUpdateAndRender(Playground, GameState->PlaygroundData, &GameState->Configuration, GameState->PlaygroundEditor, RenderGroup, Input);
            }
        } break;
        
        case LEVEL_MENU:
        {
            s32 ResultLevelIndex = PlaygroundMenuUpdateAndRender(&GameState->PlaygroundMenu, Input, RenderGroup);
            
            if (ResultLevelIndex >= 0)
            {
                RestartLevelEntity(Playground);
                GameState->PlaygroundIndex = ResultLevelIndex;
                
                playground_data *PlaygroundData = GameState->PlaygroundData;
                Assert(PlaygroundData);
                
                PrepareNextPlayground(Playground, &GameState->Configuration, PlaygroundData, ResultLevelIndex);
                
                char LevelString[3] = {};
                sprintf(LevelString, "%d", Playground->LevelNumber);
                if (Playground->LevelNumberTexture)
                {
                    SDL_DestroyTexture(Playground->LevelNumberTexture);
                }
                
                Playground->LevelNumberTexture = MakeTextureFromString(Buffer, GameState->Font, LevelString, {255, 255, 255, 255});
                Assert(Playground->LevelNumberTexture);
                
                GameState->CurrentMode = PLAYGROUND;
            }
        } break;
        
        case MAIN_MENU:
        {
            
        } break;
    }
    
    RenderGroupToOutput(RenderGroup, Buffer);
    TransState->TransGroup = TemporaryMemory;
    
    //game_editor *GameEditor = GameState->GameEditor;
    //GameEditorUpdateAndRender(Buffer, GameState, Memory, Input, GameEditor, Playground, MenuEntity);
    
    return(ShouldQuit);
}
