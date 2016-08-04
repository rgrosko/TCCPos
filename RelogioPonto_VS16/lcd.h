
#ifndef LCD_H_
#define LCD_H_

void LCD_Init (void);
void LCD_Write (char * txt, int row, int col);
void LCD_Clear (void);
void LCD_Cursor (uint32_t on);
char *IntToStr(int value, char *out);

#endif /* LCD_H_ */

