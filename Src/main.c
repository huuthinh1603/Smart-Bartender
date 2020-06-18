#include "stdio.h"
#include "string.h"
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "rw_eeprom.h"
#include "LCD_5110.h"

#define ADD_EEPROM 			0x08080010
#define CT_1				3.2
#define	CT_2				7.0 
#define CT_3				3.0
#define CT_4				2.9

extern TIM_HandleTypeDef htim2;

static int msTime;

typedef enum {
	FAIL = 0,
	TRUE,
} bool_t;

typedef enum {
	A = 0,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Space,
	Del,
	Save,
} Char_t;

typedef struct {
	bool_t Right;
	bool_t Left;
	bool_t Up;
	bool_t Down;
	bool_t Select;
} Direction_t;

typedef struct {
	Char_t Position;
	Direction_t Direction;
} KeyBoard_t;

typedef enum {
	pBottle_1 = 0,
	pBottle_2,
	pBottle_3,
	pBottle_4,
} Bottle_t;

typedef enum {
	EnterRecipe = 0,
	ListRecipe,
	AddRecipe,
	EditRecipe,
	DelRecipe,
} MainMenu_t;

 typedef struct {
	char Name[11];
	int Bottle_1;
	int Bottle_2;
	int Bottle_3;
	int Bottle_4;
} RecipeDrink_t;

/* Structure EnterRecipe */
typedef struct {
	bool_t FlagRecipe;
	bool_t FlagAction;
	Bottle_t PositionBottle;
	RecipeDrink_t Recipe;
} EnterRecipe_t;


/* ListRecipe Structures */
typedef struct {
	bool_t FlagPosition;
	bool_t FlagAction;
	bool_t FlagRecipe;
	bool_t FlagRecipeExist[30];
	int PositionRecipe;
	Bottle_t PositionBottle;
	RecipeDrink_t Recipe[30];
	RecipeDrink_t RecipeDrink_Temp;
	int MainPositionRecipe;
	int ShowPositionRecipe[4];
} ListRecipe_t;

/* AddRecipe Structures */
typedef struct {
	bool_t FlagPositionRecipe;
	bool_t FlagSave;
	bool_t FlagRecipe;
	bool_t FlagRecipeName;
	bool_t FlagRecipeExist[30];
	int PositionRecipe;
	Bottle_t PositionBottle;
	RecipeDrink_t Recipe[30];
	RecipeDrink_t RecipeDrink_Temp;
	KeyBoard_t KeyBoard;
	int MainPositionRecipe;
	int ShowPositionRecipe[4];
} AddRecipe_t;

/* EditRecipe Structures */
typedef struct {
	bool_t FlagPosition;
	bool_t FlagEdit;
	bool_t FlagRecipe;
	bool_t FlagRecipeExist[30];
	int PositionRecipe;
	Bottle_t PositionBottle;
	RecipeDrink_t Recipe[30];
	RecipeDrink_t RecipeDrink_Temp;
	int MainPositionRecipe;
	int ShowPositionRecipe[4];
} EditRecipe_t;

/* DelRecipe Structures */
typedef struct {
	bool_t FlagPosition;
	bool_t FlagDel;
	bool_t FlagRecipeExist[30];
	RecipeDrink_t Recipe[30];
	int PositionRecipe;
	int MainPositionRecipe;
	int ShowPositionRecipe[4];
} DelRecipe_t;

typedef struct {
	MainMenu_t Position; /* Current position */	
	EnterRecipe_t EnterRecipe;
	ListRecipe_t ListRecipe;
	AddRecipe_t AddRecipe;
	EditRecipe_t EditRecipe;
	DelRecipe_t DelRecipe;
	bool_t FlagAction;
	MainMenu_t MainMenu[3]; 
	int PointMain;
} ControlSystem_t;


static ControlSystem_t ControlSystem;

char Buf[28][2] = {"A", "B", "C", "D", "E", "F",
					"G", "H", "I", "J", "K", "L", 
					"M", "N", "O", "P", "Q", "R",
					"S", "T", "U", "V", "W",
					"X", "Y", "Z", "_", "x"
};

void SystemClock_Config(void);

void RecipeInit(RecipeDrink_t *Recipe, int Bottle_1, 
		int Bottle_2, int Bottle_3, int Bottle_4)
{
	Recipe->Bottle_1 = Bottle_1;
	Recipe->Bottle_2 = Bottle_2;
	Recipe->Bottle_3 = Bottle_3;
	Recipe->Bottle_4 = Bottle_4;	
}

void SaveRecipeIntoEEPROM(RecipeDrink_t Recipe, int Position) 
{
	EEPROM_Write((ADD_EEPROM + Position * 16), '@');
	for(int i = 0; i < 11; i++)
		EEPROM_Write((ADD_EEPROM + Position * 16 + 1 + i), Recipe.Name[i]);
	
	EEPROM_Write((ADD_EEPROM + Position * 16 + 12), Recipe.Bottle_1);
	EEPROM_Write((ADD_EEPROM + Position * 16 + 13), Recipe.Bottle_2);
	EEPROM_Write((ADD_EEPROM + Position * 16 + 14), Recipe.Bottle_3);
	EEPROM_Write((ADD_EEPROM + Position * 16 + 15), Recipe.Bottle_4);
}

void GetRecipeFromEEPROM(RecipeDrink_t *Recipe, bool_t *FlagRecipeExist, int Position)
{	
	if(EEPROM_Read(ADD_EEPROM + (Position * 16)) == '@') {
		for(int i = 0; i < 11; i++)
			Recipe[Position].Name[i] = EEPROM_Read(ADD_EEPROM + (Position * 16) + i + 1);
		
		Recipe[Position].Bottle_1 = EEPROM_Read(ADD_EEPROM + (Position * 16) + 12);
		Recipe[Position].Bottle_2 = EEPROM_Read(ADD_EEPROM + (Position * 16) + 13);
		Recipe[Position].Bottle_3 = EEPROM_Read(ADD_EEPROM + (Position * 16) + 14);
		Recipe[Position].Bottle_4 = EEPROM_Read(ADD_EEPROM + (Position * 16) + 15);
		FlagRecipeExist[Position] = TRUE;
	} else {
		FlagRecipeExist[Position] = FAIL;
		Recipe[Position].Bottle_1 = 0;
		Recipe[Position].Bottle_2 = 0;
		Recipe[Position].Bottle_3 = 0;
		Recipe[Position].Bottle_4 = 0;
	}
}

void ListRecipeInit(int *ShowPositionRecipe, int *MainPositionRecipe)
{
	*MainPositionRecipe = 0;
	ShowPositionRecipe[0] = 0;
	ShowPositionRecipe[1] = 1;
	ShowPositionRecipe[2] = 2;
	ShowPositionRecipe[3] = 3;	
}
void ControlSystemInit(ControlSystem_t *ControlSystem)
{	
	ControlSystem->Position = EnterRecipe;
	ControlSystem->FlagAction = TRUE;
	ControlSystem->PointMain = 0;
	ControlSystem->MainMenu[0] = EnterRecipe;
	ControlSystem->MainMenu[1] = ListRecipe;
	ControlSystem->MainMenu[2] = AddRecipe;
	
	/* Init EnterRecipe */
	ControlSystem->EnterRecipe.FlagRecipe = FAIL;
	ControlSystem->EnterRecipe.FlagAction = FAIL;
	RecipeInit(&ControlSystem->EnterRecipe.Recipe, 0, 0, 0, 200);
	
	/* Init AddRecipe */
	ControlSystem->AddRecipe.FlagPositionRecipe = FAIL;
	ControlSystem->AddRecipe.FlagRecipe = FAIL;
	ControlSystem->AddRecipe.FlagSave = FAIL;
	ControlSystem->AddRecipe.PositionRecipe = 0;
	ControlSystem->AddRecipe.PositionBottle = pBottle_1;
	RecipeInit(&ControlSystem->AddRecipe.RecipeDrink_Temp, 50, 50, 50, 50);
	ListRecipeInit(ControlSystem->AddRecipe.ShowPositionRecipe, &ControlSystem->AddRecipe.MainPositionRecipe);
	
	/* Init ListRecipe */
	ControlSystem->ListRecipe.FlagAction = FAIL;
	ControlSystem->ListRecipe.FlagPosition = FAIL;
	ControlSystem->ListRecipe.FlagRecipe = FAIL;
	ControlSystem->ListRecipe.PositionBottle = pBottle_1;
	ControlSystem->ListRecipe.PositionRecipe = 0;
	ListRecipeInit(ControlSystem->ListRecipe.ShowPositionRecipe, &ControlSystem->ListRecipe.MainPositionRecipe);
	
	
	/* Init EditRecipe */
	ControlSystem->EditRecipe.FlagEdit = FAIL;
	ControlSystem->EditRecipe.FlagPosition = FAIL;
	ControlSystem->EditRecipe.FlagRecipe = FAIL;
	ControlSystem->EditRecipe.PositionBottle = pBottle_1;
	ControlSystem->EditRecipe.PositionRecipe = 0;
	ListRecipeInit(ControlSystem->EditRecipe.ShowPositionRecipe, &ControlSystem->EditRecipe.MainPositionRecipe);
	
	/* Init DelRecipe */
	ControlSystem->DelRecipe.FlagDel = FAIL;
	ControlSystem->DelRecipe.FlagPosition = FAIL;
	ControlSystem->DelRecipe.PositionRecipe = 0;
	ListRecipeInit(ControlSystem->DelRecipe.ShowPositionRecipe, &ControlSystem->DelRecipe.MainPositionRecipe);
	
	
	/* Setting LCD */
	LCD_SetRST(GPIOB, GPIO_PIN_13);
	LCD_SetCE(GPIOB, GPIO_PIN_14);
	LCD_SetDC(GPIOB, GPIO_PIN_15);
	LCD_SetDIN(GPIOB, GPIO_PIN_1);
	LCD_SetCLK(GPIOB, GPIO_PIN_2);
	LCD_Init();
	
	/* Start Timer */
	HAL_TIM_Base_Start_IT(&htim2);
}


void DrinkBegin(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);		
}

int GetmsTime(void)
{
	return msTime;
}
void DrinkAction(RecipeDrink_t Recipe, bool_t *FlagAction)
{
	int TimeBegin[4], TimeAction[4];
	int Flag[4] = {TRUE, TRUE, TRUE, TRUE};
	int i = 0;
	
	TimeBegin[0] = TimeBegin[1] = TimeBegin[2] = TimeBegin[3] = GetmsTime();
	TimeAction[0] = (Recipe.Bottle_1 / CT_1) * 1000;
	TimeAction[1] = (Recipe.Bottle_2 / CT_2) * 1000;
	TimeAction[2] = (Recipe.Bottle_3 / CT_3) * 1000;
	TimeAction[3] = (Recipe.Bottle_4 / CT_4) * 1000;
	
	DrinkBegin();
	do {
		for(i = 0; i < 4; i++) {
			if((GetmsTime() - TimeBegin[i]) >= TimeAction[i]) {
				if(i == 0 && Flag[i] == TRUE) {
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
					Flag[i] = FAIL;
				} else if(i == 1 && Flag[i] == TRUE) {
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
					Flag[i] = FAIL;
				} else if(i == 2 && Flag[i] == TRUE) {
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
					Flag[i] = FAIL;
				} else if(i == 3 && Flag[i] == TRUE) {
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
					Flag[i] = FAIL;
				}
			}
		}
		if(*FlagAction == FAIL) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
			break;
		}
	} while((Flag[0] || Flag[1] || Flag[2] || Flag[3]));
	
}

int NumberRound(int Round, int HS)
{
	do {
		if(Round < 0)
			Round = Round + HS;
		else if(Round >= HS)
			Round = Round - HS;
	}while(Round < 0 && Round >=  HS);
	
	return Round;
}
void __MainMenu(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->Position != ControlSystem->MainMenu[ControlSystem->PointMain]) {
		if(ControlSystem->Position == (MainMenu_t)(NumberRound((ControlSystem->MainMenu[ControlSystem->PointMain] - 1), 5))) {
			if(ControlSystem->PointMain == 0) {
				for(int i = 0; i < 3; i++)
					ControlSystem->MainMenu[i] = (MainMenu_t)(NumberRound((ControlSystem->MainMenu[i] - 1), 5));
			} else 
				ControlSystem->PointMain = ControlSystem->PointMain--;
		} else if(ControlSystem->Position == (MainMenu_t)(NumberRound((ControlSystem->MainMenu[ControlSystem->PointMain] + 1), 5))) {
			if(ControlSystem->PointMain == 2) {
				for(int i = 0; i < 3; i++)
					ControlSystem->MainMenu[i] = (MainMenu_t)(NumberRound((ControlSystem->MainMenu[i] + 1), 5));
			} else 
				ControlSystem->PointMain = ControlSystem->PointMain++;
		}
	}
}

void MainMenu(ControlSystem_t *ControlSystem)
{
	__MainMenu(ControlSystem);
	
	LCD_clrScr();
	for(int i = 0; i < 3; i++) {
		if(ControlSystem->Position == EnterRecipe) {
			if(ControlSystem->MainMenu[i] == EnterRecipe) {
				LCD_InvertText(true);
				LCD_Print("> Enter Recipe", 0, i * 2);
				LCD_InvertText(false);
				continue;
			}
		} else if(ControlSystem->Position == ListRecipe) {
			if(ControlSystem->MainMenu[i] == ListRecipe) {
				LCD_InvertText(true);
				LCD_Print("> List Recipe", 0, i * 2);
				LCD_InvertText(false);
				continue;
			}
		} else if(ControlSystem->Position == AddRecipe) {
			if(ControlSystem->MainMenu[i] == AddRecipe) {
				LCD_InvertText(true);
				LCD_Print("> Add Recipe", 0, i * 2);
				LCD_InvertText(false);
				continue;
			}
		} else if(ControlSystem->Position == EditRecipe) {
			if(ControlSystem->MainMenu[i] == EditRecipe) {
				LCD_InvertText(true);
				LCD_Print("> Edit Recipe", 0, i * 2);
				LCD_InvertText(false);
				continue;
			}
		} else if(ControlSystem->Position == DelRecipe) {
			if(ControlSystem->MainMenu[i] == DelRecipe) {
				LCD_InvertText(true);
				LCD_Print("> Del Recipe", 0, i * 2);
				LCD_InvertText(false);
				continue;
			}
		}
		
		if(ControlSystem->MainMenu[i] == EnterRecipe)
			LCD_Print("> Enter Recipe", 0, i * 2);
		else if(ControlSystem->MainMenu[i] == ListRecipe)
			LCD_Print("> List Recipe", 0, i * 2);
		else if(ControlSystem->MainMenu[i] == AddRecipe)
			LCD_Print("> Add Recipe", 0, i * 2);
		else if(ControlSystem->MainMenu[i] == EditRecipe)
			LCD_Print("> Edit Recipe", 0, i * 2);
		else if(ControlSystem->MainMenu[i] == DelRecipe)
			LCD_Print("> Del Recipe", 0, i * 2);	
	}
}

void ShowRecipe(RecipeDrink_t Recipe, Bottle_t Position)
{
	char Buf[4][20];
	
	for(int i = 0; i < 4; i++)
		memset(Buf[i], '\0', 20);
		
	/* Convert integer to sting */
	sprintf(Buf[0], "Botl 1:  %d   ", Recipe.Bottle_1);
	sprintf(Buf[1], "Botl 2:  %d   ", Recipe.Bottle_2);
	sprintf(Buf[2], "Botl 3:  %d   ", Recipe.Bottle_3);
	sprintf(Buf[3], "Botl 4:  %d   ", Recipe.Bottle_4);
	
	LCD_clrScr();
	for(int i = 0; i < 4; i++) {
		if(Position == (Bottle_t)i) {
			LCD_InvertText(true);
			LCD_Print(Buf[i], 0, i + 2);
			LCD_InvertText(false);
		}else {
			LCD_Print(Buf[i], 0, i + 2);
		}
	}
}



void __PositionRecipe(int Position, int *MainPosition, int *ShowPositionRecipe)
{
	if(Position != ShowPositionRecipe[*MainPosition]) {
		if(Position == NumberRound(ShowPositionRecipe[*MainPosition] - 1, 30)) {
			if(*MainPosition == 0) {
				for(int i = 0; i < 4; i++)
					ShowPositionRecipe[i] = NumberRound(ShowPositionRecipe[i] - 1, 30);
			} else 
				*MainPosition = *MainPosition - 1;
		}else if(Position == NumberRound(ShowPositionRecipe[*MainPosition] + 1, 30)) {
			if(*MainPosition == 3) {
				for(int i = 0; i < 4; i++)
					ShowPositionRecipe[i] = NumberRound(ShowPositionRecipe[i] + 1, 30);
			} else 
				*MainPosition = *MainPosition + 1;
		}
	}
}
void PositionRecipe(RecipeDrink_t Recipe[], bool_t RecipeExist[], int Position, 
				int *MainPosition, int *ShowPositionRecipe)
{ 
	char Buf[4][15];
	
	__PositionRecipe(Position, MainPosition, ShowPositionRecipe);
	for(int i = 0; i < 4; i++) {
		memset(Buf[i], '\0', 15);
		if(RecipeExist[ShowPositionRecipe[i]] == TRUE)
			sprintf(Buf[i], "%d. %s", ShowPositionRecipe[i], Recipe[ShowPositionRecipe[i]].Name);
		else
			sprintf(Buf[i], "%d. %s", ShowPositionRecipe[i], "---------");
	}
		
	LCD_clrScr();
	for(int i = 0; i < 4; i++) {
		if(ShowPositionRecipe[i] == Position) {
			LCD_InvertText(true);
			LCD_Print(Buf[i], 0, i + 2);
			LCD_InvertText(false);
		} else
			LCD_Print(Buf[i], 0, i + 2);
	}
}

void __EnterRecipeName(KeyBoard_t *KeyBoard, char *RecipeName, bool_t *FlagSave)
{
	int Leng = 0;
	
	if(KeyBoard->Direction.Left == TRUE) {
		if((int)(KeyBoard->Position - 1) < 0)
			KeyBoard->Position = A;
		else
			KeyBoard->Position = (Char_t)(KeyBoard->Position - 1);
		
		KeyBoard->Direction.Left = FAIL;
	}
	
	if(KeyBoard->Direction.Right == TRUE) {
		if((int)(KeyBoard->Position + 1) > Save)
			KeyBoard->Position = Save;
		else
			KeyBoard->Position = (Char_t)(KeyBoard->Position + 1);
		
		KeyBoard->Direction.Right = FAIL;
	}
	
	if(KeyBoard->Direction.Up == TRUE) {
		if((int)(KeyBoard->Position - 7) >= 0)
			KeyBoard->Position = (Char_t)(KeyBoard->Position - 7);
			
		KeyBoard->Direction.Up = FAIL;
	}
	
	if(KeyBoard->Direction.Down == TRUE) {
		if((int)(KeyBoard->Position + 7) > Save)
			KeyBoard->Position = Save;
		else
			KeyBoard->Position = (Char_t)(KeyBoard->Position + 7);
		
		KeyBoard->Direction.Down = FAIL;
	}
	
	if(KeyBoard->Direction.Select == TRUE) {
		Leng = strlen(RecipeName);
		if(KeyBoard->Position == Save)
			*FlagSave = TRUE;
		else if(KeyBoard->Position == Del && Leng > 0)
			RecipeName[Leng - 1] = '\0';
		else if(Leng < 10)
			strcat(RecipeName, Buf[KeyBoard->Position]);
		else if(Leng >= 10) {
			LCD_clrScr();
			LCD_Print("   Maxsize! ", 0, 2);
			HAL_Delay(500);			
		}
		
		KeyBoard->Direction.Select = FAIL;
	}
}

void EnterRecipeName(KeyBoard_t *KeyBoard, char *RecipeName, bool_t *FlagSave)
{
	__EnterRecipeName(KeyBoard, RecipeName, FlagSave);
	
	LCD_clrScr();
	for(int i = 0; i < 28; i++) {
		if(KeyBoard->Position == (Char_t)i) {
			LCD_InvertText(true);
			LCD_Print(*(Buf + i), ((i * 2 * 6) % 84), (int)((i * 2 * 6) / 84));
			LCD_InvertText(false);
		}else {
			LCD_Print(*(Buf + i), ((i * 2 * 6) % 84), (int)((i * 2 * 6) / 84));
		}
	}
	if(KeyBoard->Position == Save) {
		LCD_InvertText(true);
		LCD_Print("     SAVE   ", 0, 4);
		LCD_InvertText(false);
	}else 
		LCD_Print("     SAVE   ", 0, 4);
	
	LCD_Print(RecipeName, 0, 5);
}

void EnterRecipe_Handler(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagRecipe == TRUE) {
		if(ControlSystem->EnterRecipe.FlagAction == TRUE) {
			/* Print LCD "Dang pha che" */
			LCD_clrScr();
			LCD_Print("  Drinking...", 0, 2);
			DrinkAction(ControlSystem->EnterRecipe.Recipe, &ControlSystem->EnterRecipe.FlagAction);
			LCD_clrScr();
			LCD_Print("  Success!!!", 0, 2);
			HAL_Delay(500);
			ControlSystem->EnterRecipe.FlagAction = FAIL;
		} else if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
			/* Print LCD Recipe */
			ShowRecipe(ControlSystem->EnterRecipe.Recipe, ControlSystem->EnterRecipe.PositionBottle);
			LCD_Print("  Enter Recipe", 0, 0);
			ControlSystem->FlagAction = FAIL;
		}
	} else if(ControlSystem->EnterRecipe.FlagRecipe == FAIL) {
		MainMenu(ControlSystem);
		ControlSystem->FlagAction = FAIL;
	}	
}
void AddRecipe_Handler(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
		if(ControlSystem->AddRecipe.FlagRecipe == FAIL) {
			/* Print LCD Position */
			PositionRecipe(ControlSystem->AddRecipe.Recipe, ControlSystem->AddRecipe.FlagRecipeExist, 
			   ControlSystem->AddRecipe.PositionRecipe, &ControlSystem->AddRecipe.MainPositionRecipe,
				ControlSystem->AddRecipe.ShowPositionRecipe);
			
			LCD_Print("  Add Recipe", 0, 0);
			if(ControlSystem->AddRecipe.PositionRecipe == ControlSystem->AddRecipe.ShowPositionRecipe[ControlSystem->AddRecipe.MainPositionRecipe])
				ControlSystem->FlagAction = FAIL;
		} else if(ControlSystem->AddRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipeExist[ControlSystem->AddRecipe.PositionRecipe] == FAIL) {
				if(ControlSystem->AddRecipe.FlagRecipeName == TRUE) {
					if(ControlSystem->AddRecipe.FlagSave == TRUE) {
						/* Print LCD "Dang luu cong thuc" */
						LCD_clrScr();
						LCD_Print("  Saving...", 0, 2);
						SaveRecipeIntoEEPROM(ControlSystem->AddRecipe.RecipeDrink_Temp, ControlSystem->AddRecipe.PositionRecipe);
						LCD_clrScr();
						LCD_Print("  Success!!!", 0, 2);
						HAL_Delay(500);
						GetRecipeFromEEPROM(ControlSystem->AddRecipe.Recipe, ControlSystem->AddRecipe.FlagRecipeExist, ControlSystem->AddRecipe.PositionRecipe);
						memset(ControlSystem->AddRecipe.RecipeDrink_Temp.Name, '\0', 11);
						RecipeInit(&ControlSystem->AddRecipe.RecipeDrink_Temp, 50, 50, 50, 50);
						ControlSystem->AddRecipe.KeyBoard.Position = A;
						ControlSystem->AddRecipe.PositionBottle = pBottle_1;
						ControlSystem->AddRecipe.FlagSave = FAIL;
						ControlSystem->AddRecipe.FlagRecipeName = FAIL;
						ControlSystem->AddRecipe.FlagRecipe = FAIL;
					} else if(ControlSystem->AddRecipe.FlagSave == FAIL) {
						/* Enter the recipe name */
						EnterRecipeName(&ControlSystem->AddRecipe.KeyBoard, 
							ControlSystem->AddRecipe.RecipeDrink_Temp.Name, &ControlSystem->AddRecipe.FlagSave);
						
						if(ControlSystem->AddRecipe.FlagSave == FAIL)
							ControlSystem->FlagAction = FAIL;
					}
				} else if(ControlSystem->AddRecipe.FlagRecipeName == FAIL) {
					/* Enter recipe */
					ShowRecipe(ControlSystem->AddRecipe.RecipeDrink_Temp, ControlSystem->AddRecipe.PositionBottle);
					LCD_Print("  Add Recipe", 0, 0);
					ControlSystem->FlagAction = FAIL;
				}
			} else if(ControlSystem->AddRecipe.FlagRecipeExist[ControlSystem->AddRecipe.PositionRecipe] == TRUE) {
				LCD_clrScr();
				LCD_Print("   Recipe is       exist!", 0, 2);
				HAL_Delay(500);
				ControlSystem->AddRecipe.FlagRecipe = FAIL;
				
			}
		}
	} else if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL) {
		MainMenu(ControlSystem);
		ControlSystem->FlagAction = FAIL;
	}		
}
void ListRecipe_Handler(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
		if(ControlSystem->ListRecipe.FlagRecipe == FAIL) {
			/* Print LCD Position */
			PositionRecipe(ControlSystem->ListRecipe.Recipe, ControlSystem->ListRecipe.FlagRecipeExist,ControlSystem->ListRecipe.PositionRecipe,
			&ControlSystem->ListRecipe.MainPositionRecipe, ControlSystem->ListRecipe.ShowPositionRecipe);
			LCD_Print("  List Recipe", 0, 0);
			if(ControlSystem->ListRecipe.PositionRecipe == ControlSystem->ListRecipe.ShowPositionRecipe[ControlSystem->ListRecipe.MainPositionRecipe])
				ControlSystem->FlagAction = FAIL;
		} else if(ControlSystem->ListRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipeExist[ControlSystem->ListRecipe.PositionRecipe] == FAIL) {
				/* Print LCD "Cong thuc khong ton tai!" */
				LCD_clrScr();
				LCD_Print(" Recipe isn't      exist", 0, 2);
				HAL_Delay(500);
				ControlSystem->ListRecipe.FlagRecipe = FAIL;
			} else if(ControlSystem->ListRecipe.FlagRecipeExist[ControlSystem->ListRecipe.PositionRecipe] == TRUE) {
				if(ControlSystem->ListRecipe.FlagAction == TRUE) {
					LCD_clrScr();
					LCD_Print("  Drinking...", 0, 2);
					DrinkAction(ControlSystem->ListRecipe.RecipeDrink_Temp, &ControlSystem->ListRecipe.FlagAction);
					LCD_clrScr();
					LCD_Print("  Success!!!", 0, 2);
					HAL_Delay(500);
					ControlSystem->ListRecipe.FlagAction = FAIL;
				} else if(ControlSystem->ListRecipe.FlagAction == FAIL) {
					/* Print LCD Recipe */
					ShowRecipe(ControlSystem->ListRecipe.RecipeDrink_Temp, ControlSystem->ListRecipe.PositionBottle);
					LCD_Print("  List Recipe", 0, 0);
					ControlSystem->FlagAction = FAIL;
				}
			}
		}
	} else if(ControlSystem->ListRecipe.FlagPosition == FAIL) {
		MainMenu(ControlSystem);
		ControlSystem->FlagAction = FAIL;
	}		
}
void EditRecipe_Handler(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
		if(ControlSystem->EditRecipe.FlagRecipe == FAIL) {
			/* Print LCD Position */
			PositionRecipe(ControlSystem->EditRecipe.Recipe, ControlSystem->EditRecipe.FlagRecipeExist, ControlSystem->EditRecipe.PositionRecipe,
				&ControlSystem->EditRecipe.MainPositionRecipe, ControlSystem->EditRecipe.ShowPositionRecipe);
			LCD_Print("  Edit Recipe", 0, 0);
			if(ControlSystem->EditRecipe.PositionRecipe == ControlSystem->EditRecipe.ShowPositionRecipe[ControlSystem->EditRecipe.MainPositionRecipe])
				ControlSystem->FlagAction = FAIL;
		} else if(ControlSystem->EditRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipeExist[ControlSystem->EditRecipe.PositionRecipe] == FAIL) {
				/* Print LCD "The recipe isn't exist" */
				LCD_clrScr();
				LCD_Print("  Recipe isn't      exist", 0, 2);
				HAL_Delay(500);
				ControlSystem->EditRecipe.FlagRecipe = FAIL;
			} else if(ControlSystem->EditRecipe.FlagRecipeExist[ControlSystem->EditRecipe.PositionRecipe] == TRUE) {
				if(ControlSystem->EditRecipe.FlagEdit == TRUE) {
					LCD_clrScr();
					LCD_Print("  Editing...", 0, 2);
					SaveRecipeIntoEEPROM(ControlSystem->EditRecipe.RecipeDrink_Temp, ControlSystem->EditRecipe.PositionRecipe);
					GetRecipeFromEEPROM(ControlSystem->EditRecipe.Recipe, ControlSystem->EditRecipe.FlagRecipeExist, ControlSystem->EditRecipe.PositionRecipe);
					LCD_clrScr();
					LCD_Print("  Success!!!", 0, 2);
					HAL_Delay(500);
					ControlSystem->EditRecipe.FlagEdit = FAIL;
					ControlSystem->EditRecipe.FlagRecipe = FAIL;
				} else if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
					/* Print LCD Recipe */
					ShowRecipe(ControlSystem->EditRecipe.RecipeDrink_Temp, ControlSystem->EditRecipe.PositionBottle);
					LCD_Print("  Edit Recipe", 0, 0);
					ControlSystem->FlagAction = FAIL;
				}					
			}
		}
	} else if(ControlSystem->EditRecipe.FlagPosition == FAIL) {
		MainMenu(ControlSystem);
		ControlSystem->FlagAction = FAIL;
	}		
}
void DelRecipe_Handler(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagPosition == TRUE) {
		if(ControlSystem->DelRecipe.FlagDel == TRUE) {
			if(ControlSystem->DelRecipe.FlagRecipeExist[ControlSystem->DelRecipe.PositionRecipe] == TRUE) {
				/* Print LCD "Dang xoa cong thuc" */
				LCD_clrScr();
				LCD_Print("  Deleting...", 0, 2);
				HAL_Delay(300);
				EEPROM_Write((ADD_EEPROM + ControlSystem->DelRecipe.PositionRecipe * 16), '$');
				LCD_clrScr();
				LCD_Print("  Success!!!", 0, 2);
				HAL_Delay(500);
				GetRecipeFromEEPROM(ControlSystem->DelRecipe.Recipe, ControlSystem->DelRecipe.FlagRecipeExist, ControlSystem->DelRecipe.PositionRecipe);
				ControlSystem->DelRecipe.FlagDel = FAIL;
			} else if(ControlSystem->DelRecipe.FlagRecipeExist[ControlSystem->DelRecipe.PositionRecipe] == FAIL) {
				/* Print LCD "Cong thuc khong ton tai" */
				LCD_clrScr();
				LCD_Print("  Recipe isn't      exist", 0, 2);
				HAL_Delay(500);
				ControlSystem->DelRecipe.FlagDel = FAIL;
			}
		} else if(ControlSystem->DelRecipe.FlagDel == FAIL) {
			/* Print LCD Position */
			PositionRecipe(ControlSystem->DelRecipe.Recipe, ControlSystem->DelRecipe.FlagRecipeExist,
				ControlSystem->DelRecipe.PositionRecipe, &ControlSystem->DelRecipe.MainPositionRecipe,
				ControlSystem->DelRecipe.ShowPositionRecipe);
			LCD_Print(" Delete Recipe", 0, 0);
			ControlSystem->FlagAction = FAIL;
	
		}
		
	} else if(ControlSystem->DelRecipe.FlagPosition == FAIL) {
		MainMenu(ControlSystem);
		ControlSystem->FlagAction = FAIL;
	}
}
int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM2_Init();
	ControlSystemInit(&ControlSystem);

	
	while (1)
	{
		if(ControlSystem.FlagAction == TRUE) {
			if(ControlSystem.Position == EnterRecipe)		/* EnterRecipe */
				EnterRecipe_Handler(&ControlSystem);
			else if(ControlSystem.Position == AddRecipe)	/* AddRecipe */
				AddRecipe_Handler(&ControlSystem);
			else if(ControlSystem.Position == ListRecipe)	/* ListRecipe */
				ListRecipe_Handler(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)	/* EditRecipe */		
				EditRecipe_Handler(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)	/* DelRecipe */
				DelRecipe_Handler(&ControlSystem);
		}
		
	}
		
}

/* ------------------------- Select ----------------------------- */
void Select_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
		if(ControlSystem->EnterRecipe.FlagRecipe == FAIL)
			ControlSystem->EnterRecipe.FlagRecipe = TRUE;
		else if(ControlSystem->EnterRecipe.FlagRecipe == TRUE)
				ControlSystem->EnterRecipe.FlagAction = TRUE;
	}
}

void Select_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL) {
			/* Update the list recipe */
			for(int i = 0; i < 30; i++)
				GetRecipeFromEEPROM(ControlSystem->AddRecipe.Recipe, ControlSystem->AddRecipe.FlagRecipeExist, i);			
		
			ControlSystem->AddRecipe.FlagPositionRecipe = TRUE;
		} else if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == FAIL)
				ControlSystem->AddRecipe.FlagRecipe = TRUE;
			else if(ControlSystem->AddRecipe.FlagRecipe == TRUE 
					&& ControlSystem->AddRecipe.FlagRecipeExist[ControlSystem->AddRecipe.PositionRecipe] == FAIL) {
				if(ControlSystem->AddRecipe.FlagRecipeName == TRUE) {
					ControlSystem->AddRecipe.KeyBoard.Direction.Select = TRUE;
				} else if(ControlSystem->AddRecipe.FlagRecipeName == FAIL) {
					ControlSystem->AddRecipe.FlagRecipeName = TRUE;
				}
			}
			
		}
	}
}

void Select_ListRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagAction == FAIL) {
		if(ControlSystem->ListRecipe.FlagPosition == FAIL) {
			/* Update the list recipe */
			for(int i = 0; i < 30; i++)
				GetRecipeFromEEPROM(ControlSystem->ListRecipe.Recipe, ControlSystem->ListRecipe.FlagRecipeExist, i);
			
			ControlSystem->ListRecipe.FlagPosition = TRUE;
		} else if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipe == FAIL) {
			/* Update a recipe at n */
				ControlSystem->ListRecipe.RecipeDrink_Temp = ControlSystem->ListRecipe.Recipe[ControlSystem->ListRecipe.PositionRecipe];
				ControlSystem->ListRecipe.FlagRecipe = TRUE;
			} else if(ControlSystem->ListRecipe.FlagRecipe == TRUE 
					&& ControlSystem->ListRecipe.FlagRecipeExist[ControlSystem->ListRecipe.PositionRecipe] == TRUE)
				ControlSystem->ListRecipe.FlagAction = TRUE;
		}
	}	
}

void Select_EditRecipe(ControlSystem_t *ControlSystem)
{
	static int i = 0;
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == FAIL) {
			/* Update the list recipe */
			for(i = 0; i < 30; i++)
				GetRecipeFromEEPROM(ControlSystem->EditRecipe.Recipe, ControlSystem->EditRecipe.FlagRecipeExist, i);
			
			ControlSystem->EditRecipe.FlagPosition = TRUE;
		} else if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == FAIL) {
			/* Update a recipe at n */
				ControlSystem->EditRecipe.RecipeDrink_Temp = ControlSystem->EditRecipe.Recipe[ControlSystem->EditRecipe.PositionRecipe];
				ControlSystem->EditRecipe.FlagRecipe = TRUE;
			} else if(ControlSystem->EditRecipe.FlagRecipe == TRUE 
						&& ControlSystem->EditRecipe.FlagRecipeExist[ControlSystem->EditRecipe.PositionRecipe] == TRUE)
					ControlSystem->EditRecipe.FlagEdit = TRUE;
		}
	}
}

void Select_DelRecipe(ControlSystem_t *ControlSystem)
{
	static int i = 0;
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == FAIL) {
			for(i = 0; i < 30; i++) 
				GetRecipeFromEEPROM(ControlSystem->DelRecipe.Recipe, ControlSystem->DelRecipe.FlagRecipeExist, i);
				
			ControlSystem->DelRecipe.FlagPosition = TRUE;
		} else if(ControlSystem->DelRecipe.FlagPosition == TRUE)
			ControlSystem->DelRecipe.FlagDel = TRUE;
	}
}

/* ------------------------- Left ------------------------------- */
void Left_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
		if(ControlSystem->EnterRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->EnterRecipe.PositionBottle == pBottle_1) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_1 > 4) 
					ControlSystem->EnterRecipe.Recipe.Bottle_1 = ControlSystem->EnterRecipe.Recipe.Bottle_1 - 5;							
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_2) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_2 > 4)
					ControlSystem->EnterRecipe.Recipe.Bottle_2 = ControlSystem->EnterRecipe.Recipe.Bottle_2 - 5;
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_3) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_3 > 4)
					ControlSystem->EnterRecipe.Recipe.Bottle_3 = ControlSystem->EnterRecipe.Recipe.Bottle_3 - 5;
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_4) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_4 > 4)
					ControlSystem->EnterRecipe.Recipe.Bottle_4 = ControlSystem->EnterRecipe.Recipe.Bottle_4 - 5;
			}
		} else if(ControlSystem->EnterRecipe.FlagRecipe == FAIL)
				ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */
	}
}


void Left_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == FAIL) {
				ControlSystem->AddRecipe.PositionRecipe = (int)((30 + ControlSystem->AddRecipe.PositionRecipe - 1) % 30);
			} else if(ControlSystem->AddRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->AddRecipe.FlagRecipeName == FAIL) {
					if(ControlSystem->AddRecipe.PositionBottle == pBottle_1) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 > 4)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 - 5;
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_2) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 > 4)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 - 5;
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_3) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 > 4)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 - 5;							
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_4) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 > 4)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 - 5;				
					}
				} else if(ControlSystem->AddRecipe.FlagRecipeName == TRUE) {
					ControlSystem->AddRecipe.KeyBoard.Direction.Left = TRUE;
				}
			}						
		} else if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */
	}				
}

void Left_ListRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagAction == FAIL) {
		if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipe == FAIL) {
				ControlSystem->ListRecipe.PositionRecipe = (int)((30 + ControlSystem->ListRecipe.PositionRecipe - 1) % 30);
			} else if(ControlSystem->ListRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->ListRecipe.PositionBottle == pBottle_1) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 > 4)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 - 5;
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_2) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 > 4)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 - 5;
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_3) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 > 4)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 - 5;							
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_4) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 > 4)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 - 5;				
				}
			}						
		} else if(ControlSystem->ListRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */	
	}
}

void Ledt_EditRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == FAIL) {
				ControlSystem->EditRecipe.PositionRecipe = (int)((30 + ControlSystem->EditRecipe.PositionRecipe - 1) % 30);
			} else if(ControlSystem->EditRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->EditRecipe.PositionBottle == pBottle_1) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 > 4)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 - 5;
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_2) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 > 4)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 - 5;
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_3) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 > 4)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 - 5;							
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_4) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 > 4)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 - 5;				
				}
			}						
		} else if(ControlSystem->EditRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */		
		
	}
}

void Left_DelRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == TRUE)
			ControlSystem->DelRecipe.PositionRecipe = (30 + ControlSystem->DelRecipe.PositionRecipe - 1) % 30;
		else if(ControlSystem->DelRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */		
	}
}

/* ------------------------- Up ---------------------------- */
void Up_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
		if(ControlSystem->EnterRecipe.FlagRecipe == TRUE)
			ControlSystem->EnterRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->EnterRecipe.PositionBottle - 1) % 4);
		else if(ControlSystem->EnterRecipe.FlagRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */		
	}
}

void Up_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->AddRecipe.FlagRecipeName == FAIL)
					ControlSystem->AddRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->AddRecipe.PositionBottle - 1) % 4);
				else if(ControlSystem->AddRecipe.FlagRecipeName == TRUE)
					ControlSystem->AddRecipe.KeyBoard.Direction.Up = TRUE;
			} else if(ControlSystem->AddRecipe.FlagRecipe == FAIL)
				ControlSystem->AddRecipe.PositionRecipe = (int)((30 + ControlSystem->AddRecipe.PositionRecipe - 1) % 30);
		} else if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Tien */		
	}
}

void Up_ListEnter(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagAction == FAIL) {
		if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipe == TRUE)
				ControlSystem->ListRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->ListRecipe.PositionBottle - 1) % 4);
			else if(ControlSystem->ListRecipe.FlagRecipe == FAIL)
				ControlSystem->ListRecipe.PositionRecipe = (int)((30 + ControlSystem->ListRecipe.PositionRecipe - 1) % 30);
		} else if(ControlSystem->ListRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */		
	}
}

void Up_EditRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == TRUE)
				ControlSystem->EditRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->EditRecipe.PositionBottle - 1) % 4);
			else if(ControlSystem->EditRecipe.FlagRecipe == FAIL)
				ControlSystem->EditRecipe.PositionRecipe = (int)((30 + ControlSystem->EditRecipe.PositionRecipe - 1) % 30);
		} else if(ControlSystem->EditRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */
	}
}

void Up_DelRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == TRUE)
			ControlSystem->DelRecipe.PositionRecipe = (int)((30 + ControlSystem->DelRecipe.PositionRecipe - 1) % 30);
		else if(ControlSystem->DelRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position - 1) % 5); /* Lui */
	}
}



/* ---------------------- Right ------------------------ */
void Right_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
		if(ControlSystem->EnterRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->EnterRecipe.PositionBottle == pBottle_1) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_1 < 500) 
					ControlSystem->EnterRecipe.Recipe.Bottle_1 = ControlSystem->EnterRecipe.Recipe.Bottle_1 + 5;							
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_2) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_2 < 500)
					ControlSystem->EnterRecipe.Recipe.Bottle_2 = ControlSystem->EnterRecipe.Recipe.Bottle_2 + 5;
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_3) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_3 < 500)
					ControlSystem->EnterRecipe.Recipe.Bottle_3 = ControlSystem->EnterRecipe.Recipe.Bottle_3 + 5;
			} else if(ControlSystem->EnterRecipe.PositionBottle == pBottle_4) {
				if(ControlSystem->EnterRecipe.Recipe.Bottle_4 < 500)
					ControlSystem->EnterRecipe.Recipe.Bottle_4 = ControlSystem->EnterRecipe.Recipe.Bottle_4 + 5;
			}
		} else if(ControlSystem->EnterRecipe.FlagRecipe == FAIL)
				ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */
	}
}

void Right_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == FAIL) {
				ControlSystem->AddRecipe.PositionRecipe = (int)((30 + ControlSystem->AddRecipe.PositionRecipe + 1) % 30);
			} else if(ControlSystem->AddRecipe.FlagRecipe == TRUE){
				if(ControlSystem->AddRecipe.FlagRecipeName == FAIL) {
					if(ControlSystem->AddRecipe.PositionBottle == pBottle_1) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 < 500)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_1 + 5;
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_2) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 < 500)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_2 + 5;
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_3) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 < 500)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_3 + 5;							
					} else if(ControlSystem->AddRecipe.PositionBottle == pBottle_4) {
						if(ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 < 500)
							ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->AddRecipe.RecipeDrink_Temp.Bottle_4 + 5;				
					}
				} else if(ControlSystem->AddRecipe.FlagRecipeName == TRUE){
					ControlSystem->AddRecipe.KeyBoard.Direction.Right = TRUE;
				}
			}						
		} else if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */
	}				
}

void Right_ListRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagAction == FAIL) {
		if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipe == FAIL) {
				ControlSystem->ListRecipe.PositionRecipe = (int)((30 + ControlSystem->ListRecipe.PositionRecipe + 1) % 30);
			} else if(ControlSystem->ListRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->ListRecipe.PositionBottle == pBottle_1) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 < 500)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_1 + 5;
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_2) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 < 500)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_2 + 5;
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_3) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 < 500)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_3 + 5;							
				} else if(ControlSystem->ListRecipe.PositionBottle == pBottle_4) {
					if(ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 < 500)
						ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->ListRecipe.RecipeDrink_Temp.Bottle_4 + 5;				
				}
			}						
		} else if(ControlSystem->ListRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Lui */	
	}
}

void Right_EditRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == FAIL) {
				ControlSystem->EditRecipe.PositionRecipe = (int)((30 + ControlSystem->EditRecipe.PositionRecipe + 1) % 30);
			} else if(ControlSystem->EditRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->EditRecipe.PositionBottle == pBottle_1) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 < 500)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_1 + 5;
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_2) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 < 500)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_2 + 5;
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_3) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 < 500)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_3 + 5;							
				} else if(ControlSystem->EditRecipe.PositionBottle == pBottle_4) {
					if(ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 < 500)
						ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 = ControlSystem->EditRecipe.RecipeDrink_Temp.Bottle_4 + 5;				
				}
			}						
		} else if(ControlSystem->EditRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */		
		
	}
}

void Right_DelRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == TRUE)
			ControlSystem->DelRecipe.PositionRecipe = (30 + ControlSystem->DelRecipe.PositionRecipe + 1) % 30;
		else if(ControlSystem->DelRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */		
	}
}

/* --------------------------- Down ---------------------------- */
void Down_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == FAIL) {
		if(ControlSystem->EnterRecipe.FlagRecipe == TRUE)
			ControlSystem->EnterRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->EnterRecipe.PositionBottle + 1) % 4);
		else if(ControlSystem->EnterRecipe.FlagRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */		
	}
}

void Down_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->AddRecipe.FlagRecipeName == FAIL)
					ControlSystem->AddRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->AddRecipe.PositionBottle + 1) % 4);
				else if(ControlSystem->AddRecipe.FlagRecipeName == TRUE)
					ControlSystem->AddRecipe.KeyBoard.Direction.Down = TRUE;
			} else if(ControlSystem->AddRecipe.FlagRecipe == FAIL)
				ControlSystem->AddRecipe.PositionRecipe = (int)((30 + ControlSystem->AddRecipe.PositionRecipe + 1) % 30);
		} else if(ControlSystem->AddRecipe.FlagPositionRecipe == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */		
	}
}

void Down_ListRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagAction == FAIL) {
		if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
			if(ControlSystem->ListRecipe.FlagRecipe == TRUE)
				ControlSystem->ListRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->ListRecipe.PositionBottle + 1) % 4);
			else if(ControlSystem->ListRecipe.FlagRecipe == FAIL)
				ControlSystem->ListRecipe.PositionRecipe = (int)((30 + ControlSystem->ListRecipe.PositionRecipe + 1) % 30);
		} else if(ControlSystem->ListRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */		
	}
}

void Down_EditRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == TRUE)
				ControlSystem->EditRecipe.PositionBottle = (Bottle_t)((4 + ControlSystem->EditRecipe.PositionBottle + 1) % 4);
			else if(ControlSystem->EditRecipe.FlagRecipe == FAIL)
				ControlSystem->EditRecipe.PositionRecipe = (int)((30 + ControlSystem->EditRecipe.PositionRecipe + 1) % 30);
		} else if(ControlSystem->EditRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Tien */
	}
}

void Down_DelRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == TRUE)
			ControlSystem->DelRecipe.PositionRecipe = (int)((30 + ControlSystem->DelRecipe.PositionRecipe + 1) % 30);
		else if(ControlSystem->DelRecipe.FlagPosition == FAIL)
			ControlSystem->Position = (MainMenu_t)((5 + ControlSystem->Position + 1) % 5); /* Lui */
	}
}

void Back_EnterRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EnterRecipe.FlagAction == TRUE) {
		ControlSystem->EnterRecipe.FlagAction = FAIL;
	}else if(ControlSystem->EnterRecipe.FlagRecipe == TRUE) {
		ControlSystem->EnterRecipe.PositionBottle = pBottle_1;
		ControlSystem->EnterRecipe.FlagRecipe = FAIL;
		RecipeInit(&ControlSystem->EnterRecipe.Recipe, 50, 50, 50, 50);
	} 
}
void Back_AddRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->AddRecipe.FlagSave == FAIL) {
		if(ControlSystem->AddRecipe.FlagPositionRecipe == TRUE) {
			if(ControlSystem->AddRecipe.FlagRecipe == TRUE) {
				if(ControlSystem->AddRecipe.FlagRecipeName == FAIL) {
					ControlSystem->AddRecipe.PositionBottle = pBottle_1;
					RecipeInit(&ControlSystem->AddRecipe.RecipeDrink_Temp, 50, 50, 50, 50);
					memset(ControlSystem->AddRecipe.RecipeDrink_Temp.Name, '\0', 11);
					ControlSystem->AddRecipe.KeyBoard.Position = A;
					ControlSystem->AddRecipe.FlagRecipe = FAIL;
				} else if(ControlSystem->AddRecipe.FlagRecipeName == TRUE) {
					ControlSystem->AddRecipe.FlagRecipeName = FAIL;
				}
			} else if(ControlSystem->AddRecipe.FlagRecipe == FAIL) {
				ControlSystem->AddRecipe.PositionRecipe = 0;
				memset(ControlSystem->AddRecipe.RecipeDrink_Temp.Name, '\0', 11);
				ListRecipeInit(ControlSystem->AddRecipe.ShowPositionRecipe, &ControlSystem->AddRecipe.MainPositionRecipe);
				ControlSystem->AddRecipe.FlagPositionRecipe = FAIL;
			}
		}
	}		
}
void Back_ListRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->ListRecipe.FlagPosition == TRUE) {
		if(ControlSystem->ListRecipe.FlagRecipe == TRUE) {
			if(ControlSystem->ListRecipe.FlagAction == FAIL) {
				ControlSystem->ListRecipe.PositionBottle = pBottle_1;
				ControlSystem->ListRecipe.FlagRecipe = FAIL;
			} else
				ControlSystem->ListRecipe.FlagAction = FAIL;
		} else if(ControlSystem->ListRecipe.FlagRecipe == FAIL) {
			ControlSystem->ListRecipe.PositionRecipe = 0;
			ListRecipeInit(ControlSystem->ListRecipe.ShowPositionRecipe, &ControlSystem->ListRecipe.MainPositionRecipe);
			ControlSystem->ListRecipe.FlagPosition = FAIL;
		}
	}		
}

void Back_EditRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->EditRecipe.FlagEdit == FAIL) {
		if(ControlSystem->EditRecipe.FlagPosition == TRUE) {
			if(ControlSystem->EditRecipe.FlagRecipe == TRUE ) {
				ControlSystem->EditRecipe.FlagRecipe = FAIL;
				ControlSystem->EditRecipe.PositionBottle = pBottle_1;
			} else if(ControlSystem->EditRecipe.FlagRecipe == FAIL) {
				ListRecipeInit(ControlSystem->EditRecipe.ShowPositionRecipe, &ControlSystem->EditRecipe.MainPositionRecipe);
				ControlSystem->EditRecipe.PositionRecipe = 0;
				ControlSystem->EditRecipe.FlagPosition = FAIL;
			}
		}
	}		
}

void Back_DelRecipe(ControlSystem_t *ControlSystem)
{
	if(ControlSystem->DelRecipe.FlagDel == FAIL) {
		if(ControlSystem->DelRecipe.FlagPosition == TRUE) {
			ControlSystem->DelRecipe.PositionRecipe = 0;
			ListRecipeInit(ControlSystem->DelRecipe.ShowPositionRecipe, &ControlSystem->DelRecipe.MainPositionRecipe);
			ControlSystem->DelRecipe.FlagPosition = FAIL;
		}
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	switch(GPIO_Pin) {
		case GPIO_PIN_7: /* Select */
		{
			if(ControlSystem.Position == EnterRecipe)				/* EnterRecipe */
				Select_EnterRecipe(&ControlSystem);
			else if(ControlSystem.Position == AddRecipe)			/* AddRecipe */
				Select_AddRecipe(&ControlSystem);
			else if(ControlSystem.Position == ListRecipe)			/* ListRecipe */
				Select_ListRecipe(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)			/* EditRecipe */
				Select_EditRecipe(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)			/* DelRecipe */
				Select_DelRecipe(&ControlSystem);

			break;
		}
		case GPIO_PIN_8: /* Left */
		{
			if(ControlSystem.Position == EnterRecipe)			/* EnterRecipe */
				Left_EnterRecipe(&ControlSystem);
			else if(ControlSystem.Position == AddRecipe)		/* AddRecipe */
				Left_AddRecipe(&ControlSystem);	
			else if(ControlSystem.Position == ListRecipe)		/* ListRecipe */
				Left_ListRecipe(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)		/* EditRecipe */
				Ledt_EditRecipe(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)		/* DelRecipe */
				Left_DelRecipe(&ControlSystem);
			
			break;
		}
		case GPIO_PIN_9: /* Up */
		{
			if(ControlSystem.Position == EnterRecipe)			/* EnterRecipe */
				Up_EnterRecipe(&ControlSystem);
			else if(ControlSystem.Position == AddRecipe)		/* AddRecipe */ 
				Up_AddRecipe(&ControlSystem);
			else if(ControlSystem.Position == ListRecipe)		/* ListRecipe */
				Up_ListEnter(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)		/* EditRecipe */
				Up_EditRecipe(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)		/* DelRecipe  */
				Up_DelRecipe(&ControlSystem);
			
			break;
		}
		case GPIO_PIN_10: /* Right */
		{
			if(ControlSystem.Position == EnterRecipe)			/* EnterRecipe */
				Right_EnterRecipe(&ControlSystem);		
			else if(ControlSystem.Position == AddRecipe)		/* AddRecipe */
				Right_AddRecipe(&ControlSystem);			
			else if(ControlSystem.Position == ListRecipe)		/* ListRecipe */
				Right_ListRecipe(&ControlSystem);		
			else if(ControlSystem.Position == EditRecipe)		/* EditRecipe */
				Right_EditRecipe(&ControlSystem);	
			else if(ControlSystem.Position == DelRecipe)		/* DelRecipe */
				Right_DelRecipe(&ControlSystem);
			
			break;
		}
		case GPIO_PIN_11: /* Down */
		{
			
			
			if(ControlSystem.Position == EnterRecipe)			/* EnterRecipe */
				Down_EnterRecipe(&ControlSystem);	
			else if(ControlSystem.Position == AddRecipe)		/* AddRecipe */
				Down_AddRecipe(&ControlSystem);
			else if(ControlSystem.Position == ListRecipe)		/* ListRecipe */
				Down_ListRecipe(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)		/* EditRecipe */
				Down_EditRecipe(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)		/* DelRecipe*/
				Down_DelRecipe(&ControlSystem);
			
			break;
		}
		case GPIO_PIN_12: /* Back */
		{
			if(ControlSystem.Position == EnterRecipe)		/* EnterRecipe */
				Back_EnterRecipe(&ControlSystem);	
			else if(ControlSystem.Position == AddRecipe)	/* AddRecipe */
				Back_AddRecipe(&ControlSystem);
			else if(ControlSystem.Position == ListRecipe)	/* ListRecipe */
				Back_ListRecipe(&ControlSystem);
			else if(ControlSystem.Position == EditRecipe)	/* EditRecipe */
				Back_EditRecipe(&ControlSystem);
			else if(ControlSystem.Position == DelRecipe)	/* DelRecipe */
				Back_DelRecipe(&ControlSystem);
			break;
		}
		default:
		{	
			break;
		}
	}
	ControlSystem.FlagAction = TRUE;
	HAL_Delay(150);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim2.Instance) {
		if(msTime >= 0xFFFFFFFF)
			msTime = 0;
		msTime++;
	}	
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_8;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
	
}
