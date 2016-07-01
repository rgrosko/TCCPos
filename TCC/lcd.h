
#ifndef LCD_H_
#define LCD_H_

void LCD_Init (void);
void LCD_WriteCol (char * txt, int row, int col);
void LCD_Write (char * txt, int row);
void LCD_Process (void);
void LCD_Clear (void);
char *IntToStr(int value, char *s, int radix);
void stringcat(char text[20], int value, int line);

#endif /* LCD_H_ */

