#include<stdio.h>
#include<ctype.h>
#include<string.h>

//function prototypes
int iskeyword(char*);
int hex_validation(char*);
int binary_validation(char*);
int octal_validation(char*);
int float_validation(char*);
int decimal_validation(char*);

char *keywords[] = {
"auto","break","case","char","const","continue","default","do","double",
"else","enum","extern","float","for","goto","if","int","long","register",
"return","short","signed","sizeof","static","struct","switch","typedef",
"union","unsigned","void","volatile","while"
};

char stack[1000];// for paranthesis validation
int stack_line[1000];// stores line number of brackets
int top = -1;// stack pointer (top index)

char error[100][200];//to store error messages
int error_count = 0;//error count

void scanFile( char *filename)//file reading
{
    FILE *fp;
    char ch,buffer[1000];//buffer to store lexeme temporarily
    int index;//index for buffer
    int line_count=1;//line count of file

    fp = fopen(filename,"r");//opened in read mode
    if( fp == NULL)
    {
        printf("Error : file not exists\n");
        return;
    }

    while( (ch=fgetc(fp))!=EOF )
    {
        //to skip header file line
        if( ch == '#' )
        {
            while( (ch=fgetc(fp))!='\n' && ch != EOF)
            {
                continue;
            }
            line_count++;//incrementing line count because here consuming newline
            continue;
        }

        //counting lines in the program
        if( ch == '\n' )
        {
            line_count++;
            continue;
        }

        //to skip whitespaces
        if( ch == ' ' || ch == '\t')
        {
            continue;
        }

        //identifier or keyword
        if( isalpha(ch) || ch == '_' )
        {
            index=0;
            buffer[index++] = ch;

            while( isalnum(ch=fgetc(fp)) || ch == '_' )
            {
                buffer[index++] = ch;
            }
            
            buffer[index] = '\0';
            // ungetc() pushes a character back into the input stream.
            // This is used when we read one character ahead to check token boundary,
            // but that character actually belongs to the next token.
            ungetc(ch,fp);

            if(iskeyword(buffer))
            {
                printf("KEYWORD        : %s\n",buffer);
            }
            else
            {
                printf("IDENTIFIER     : %s\n",buffer);

            }
            continue;
        }

        //Numeric const
        if( isdigit(ch) || ch == '.' )
        {
            index = 0;
            buffer[index++] = ch;

            while( isdigit(ch=fgetc(fp)) || isalpha(ch) || ch == '.' )
            {
                buffer[index++] = ch;
            }
            buffer[index] = '\0';
            ungetc(ch,fp);

            if( buffer[0] == '0' && (buffer[1] == 'x' || buffer[1] == 'X' ) )//hex
            {
                if(hex_validation(buffer))
                {
                    printf("HEX CONST      : %s\n",buffer);
                    continue;
                }
                else
                {
                    // stores formatted error message in error array
                    sprintf(error[error_count++],"Line %d: Invalid hex constant '%s'\n",line_count,buffer);
                    continue;
                }
            }

            if( buffer[0] == '0' && (buffer[1] == 'b' || buffer[1] == 'B' ) )//binary
            {
                if(binary_validation(buffer))
                {
                    printf("BINARY CONST   : %s\n",buffer);
                    continue;
                }
                else
                {
                    sprintf(error[error_count++],"Line %d: Invalid binary constant '%s'\n",line_count,buffer);
                    continue;
                }
            }

            if( buffer[0] == '0' && buffer[1] != '\0' )//octal
            {
                if(octal_validation(buffer))
                {
                    printf("OCTAL CONST    : %s\n",buffer);
                    continue;
                }
                else
                {
                    sprintf(error[error_count++],"Line %d: Invalid octal constant '%s'\n",line_count,buffer);
                    continue;
                }
            }
            
            int i=0,count=0;
            while(buffer[i])
            {
                if(buffer[i]=='.')
                {
                    count++;
                }
                i++;
            }

            if(count==1)//float
            {
                if(float_validation(buffer))
                {
                    printf("FLOAT CONST    : %s\n",buffer);
                    continue;
                }
                else
                {
                    sprintf(error[error_count++],"Line %d: Invalid float constant '%s'\n",line_count,buffer);
                    continue;
                }
            }
            else if(count>1)
            {
                sprintf(error[error_count++],"Line %d: Invalid float constant '%s'\n",line_count,buffer);
                continue;
            }

            if(decimal_validation(buffer))//decimal
            {
                printf("DECIMAL CONST  : %s\n",buffer);
                continue;
            }
            else
            {
                sprintf(error[error_count++],"Line %d: Invalid decimal constant '%s'\n",line_count,buffer);
                continue;
            }
        }

        //char const
        if( ch == '\'' )
        {
            index = 0;
            buffer[index++] = ch;
            int closed = 0;
            int char_count = 0;

            while((ch=fgetc(fp))!=EOF)
            {
                if(ch=='\n')
                {
                    line_count++;
                    break;
                }

                if(ch == '\\')
                {
                    buffer[index++] = ch;
                    ch = fgetc(fp);            // take escaped char
                    buffer[index++] = ch;
                    char_count += 2;
                    continue;
                }

                buffer[index++] = ch;

                if(ch=='\'')
                {
                    closed = 1;
                    break;
                }
                char_count++;
            }
            buffer[index] = '\0';

            if(!closed)
            {
                sprintf(error[error_count++],"Line %d: Missing closing single quote for char %s\n",line_count, buffer);
            }
            else if(char_count == 0)
            {
                sprintf(error[error_count++],"Line %d: Empty character constant ''\n", line_count);
            }
            else if(char_count == 2 && buffer[1] == '\\' )
            {
                printf("CHAR CONST     : %s\n",buffer);
            }
            else if(char_count>2)
            {
                sprintf(error[error_count++],"Line %d: Multi-character constant %s\n",line_count, buffer);
            }
            else
            {
                printf("CHAR CONST     : %s\n",buffer);
            }

            continue;
        }

        //comments skips
        if( ch == '/' )
        {
            char next = fgetc(fp);

            if( next == '/' )//single line
            {
                while( (ch=fgetc(fp))!='\n' && ch != EOF )
                {
                    continue;
                }

                if( ch == '\n' )
                {
                    line_count++;
                }
                continue;
            }
            else if( next == '*' )//multi line
            {
                while( (ch=fgetc(fp))!=EOF )
                {
                    if(ch=='*')
                    {
                        next = fgetc(fp);
                        if(next == '/' )
                        {
                            break;
                        }
                    }
                    continue;
                }
                continue;
            }
            else
            {
                ungetc(next,fp);
            }
        }

        //operators
        if(ch == '+' || ch == '-' || ch == '/' || ch == '*' || ch == '=' || 
           ch == '<' || ch == '>' || ch == '%' || ch == '!' || ch == '&' ||
           ch == '|' || ch == '^' || ch == '~' || ch == '?' )
        {
            char next_ch = fgetc(fp);
            buffer[0] = ch;
            buffer[1] = next_ch;
            buffer[2] = '\0';

            //two operators
            if((ch == '=' && next_ch == '=') ||
               (ch == '+' && next_ch == '+') ||
               (ch == '-' && next_ch == '-') ||
               (ch == '>' && next_ch == '=') ||
               (ch == '<' && next_ch == '=') ||
               (ch == '!' && next_ch == '=') ||
               (ch == '-' && next_ch == '>') ||
               (ch == '&' && next_ch == '&') ||
               (ch == '|' && next_ch == '|') ||
               (ch == '<' && next_ch == '<') ||
               (ch == '>' && next_ch == '>') ||
               (ch == '+' && next_ch == '=') ||
               (ch == '-' && next_ch == '=') ||
               (ch == '*' && next_ch == '=') ||
               (ch == '/' && next_ch == '=') ||
               (ch == '%' && next_ch == '=') ||
               (ch == '&' && next_ch == '=') ||
               (ch == '|' && next_ch == '=') ||
               (ch == '^' && next_ch == '=') )

            {
                printf("OPERATOR       : %s\n",buffer);
                continue;
            }

            //single operator
            buffer[1] = '\0';
            ungetc(next_ch,fp);
            printf("OPERATOR       : %s\n",buffer);
            continue;
        }

        //symbols
        if(ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == '[' || ch == ']' ||
           ch == ';' || ch == ':' || ch ==',' )
        {
            int error_flag = 1;

            if( ch == '(' || ch == '{' || ch == '[' )
            {
                stack[++top] = ch;
                stack_line[top] = line_count;
            }
            else if( ch == ')' || ch == '}' || ch == ']' )
            {
                if( top == -1 )
                {
                    sprintf(error[error_count++],"Line %d: Unmatched closing '%c'\n", line_count, ch);
                    error_flag = 0;
                }
                else
                {
                    char open = stack[top];
                    int open_line = stack_line[top];
                    top--;

                    if( ( open == '(' && ch != ')' ) || 
                        ( open == '[' && ch != ']' ) ||
                        ( open == '{' && ch != '}' ) )
                    {
                        sprintf(error[error_count++],"Line %d: Mismatched '%c' and '%c'\n", line_count, open, ch);
                        error_flag = 0;
                    }
                }
            }

            if(error_flag)
            {
                buffer[0] = ch;
                buffer[1] = '\0';
                printf("SYMBOL         : %s\n",buffer);
            }
            continue;
        }

        //string literals
        if( ch == '"')
        {
            index = 0;
            buffer[index++] = ch;
            int closed = 0;
            int token_line = line_count;

            while( (ch=fgetc(fp))!=EOF )
            {
                if( ch == '\n')
                {
                    line_count++;
                    break;
                }
                
                buffer[index++] = ch;
                if(ch == '"')
                {
                    closed=1;
                    break;
                }
            }
            buffer[index] = '\0';
            
            if(closed)
            {
                printf("LITERAL        : %s\n",buffer);
            }
            else
            {
                sprintf(error[error_count++],"Line %d: Missing closing double quote for string %s\n",token_line, buffer);
            }
            continue;
        }
    }

    while(top != -1)
    {
        sprintf(error[error_count++],"Line %d: Unmatched opening '%c'\n", stack_line[top],stack[top]);
        top--;
    }

    if( error_count > 0 )
        printf("\nERRORS FOUND\n");
    
    for(int i=0;i<error_count;i++)
    {
        printf("%s",error[i]);
    }

    fclose(fp);
}

int iskeyword(char *buffer)
{
    for(int i=0; i<32; i++)
    {
        if(strcmp(keywords[i],buffer)==0)
            return 1;
    }
    return 0;
}

int hex_validation(char *s)
{
    int i=2;
    if( s[i]=='\0' )
    {
        return 0;
    }
    while(s[i])
    {
        char ch = tolower(s[i]);
        if( !(isdigit(ch) || (ch >= 'a' && ch <= 'f')) )
        {
            return 0;
        }
        i++;
    }
    return 1;
}
int binary_validation(char *s)
{
    int i=2;
    if( s[i]=='\0' )
    {
        return 0;
    }
    while(s[i])
    {
        if( s[i] < '0' || s[i] > '1' )
        {
            return 0;
        }
        i++;
    }
    return 1;
}
int octal_validation(char *s)
{
    int i=1;
    while(s[i])
    {
        if( s[i] < '0' || s[i] > '7' )
        {
            return 0;
        }
        i++;
    }
    return 1;
}
int float_validation(char *s)
{
    int len = strlen(s) - 1;
    if(!(isdigit(s[len]) || s[len] == 'f' || s[len] == 'F'))
    {
        return 0;
    }

    int i=0,count_f=0;
    while(s[i])
    {
        if(s[i]=='f'||s[i]=='F')
        {
            count_f++;
        }
        i++;
    }
    if(count_f>1)
    {
        return 0;
    }

    i=0;
    while(s[i])
    {
        if(!( isdigit(s[i]) || s[i] == '.' || s[i] =='f' || s[i] =='F'))
        {
            return 0;
        }
        i++;
    }
    return 1;
}
int decimal_validation(char *s)
{
    int i=0;
    while(s[i])
    {
        if( !(isdigit(s[i])) )
        {
            return 0;
        }
        i++;
    }
    return 1;
}