#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "inifile.h"

char* inifile_cleanstr(char* str)
{
	int i;
	for (i = strlen(str) - 1; i >= 0; i --)
		if (isspace(str[i]))
			str[i] = '\0';
		else
			break;
	
	while (isspace(*str))
		str++;
	
	return str;
}

bool inifile_parseint(const char* str, bool issigned, int32_t* integer)
{
	int32_t ret = 0;
	bool neg = false;
	if (str[0] == '0' && str[1] == 'x')
	{
		int i;
		if (str[2] == '\0')
			return false;
		
		for (i = 0, str += 2; *str; i++, str++)
		{
			if (i == 8)
				return false;
			
			ret <<= 4;
			if (*str >= '0' && *str <= '9')
				ret |= *str - '0';
			else if (*str >= 'a' && *str <= 'f')
				ret |= *str - 'a' + 10;
			else if (*str >= 'A' && *str <= 'F')
				ret |= *str - 'A' + 10;
			else
				return false;
		}
		*integer = ret;
		return true;
	}
	else if ((*str >= '0' && *str <= '9') || (neg = (*str == '-')))
	{
		int i;
		if (neg)
			str++;
		for (i = 0; *str; i++, str++)
		{
			if (i == 10)
				return false;
			else if (i == 9 && !issigned && (ret > 429496729LL || (ret == 429496729LL && *str > '5')))
				return false;
			else if (i == 9 && issigned && (ret > 214748364LL || (ret == 214748364LL && *str > (neg ? '8' : '7'))))
				return false;
			
			ret *= 10;
			if (*str >= '0' && *str <= '9')
				ret += *str - '0';
			else
				return false;
		}
		if (neg)
			ret *= -1;
		*integer = ret;
		return true;
	}
	else
		return false;
}

bool inifile_read(const char* filename, inifile_callback callback)
{
	FILE* fp = fopen(filename, "r");
	char* inisection = "";
	char readbuf[4096] = "";
	char* readptr;
	bool success = true;
	bool newsection = false;
	
	if (!fp)
		return inifile_cantread;
	
	while ((readptr = fgets(readbuf, sizeof(readbuf), fp))) // Loop through each line.
	{
		while (isspace(readptr[0])) // Skip whitespace.
			readptr++;
		
		if (readptr[0] == '\0' || readptr[0] == '#' || readptr[0] == '!') // Skip empty lines and comments.
			continue;
		else if (readptr[0] == '[' && readptr[1] != ']') // It's a section header.
		{
			int i;
			for (i = 2; readptr[i]; i ++) // Look for the ]
				if (readptr[i] == ']')
					break;
			
			if (readptr[i]) // Replace with a null or crash with error.
				readptr[i] = '\0';
			else
			{
				success = false;
				break;
			}
			
			if (inisection[0])
				free(inisection);
			inisection = strdup(readptr + 1); // Skip the first [
			newsection = true;
		}
		else // It's a value.
		{
			int i;
			int equals = 0;
			for (i = 0; readptr[i]; i ++) // Find the =
				if (readptr[i] == '=')
					equals = i;
			
			if (readptr[i - 1] == '\n')
				readptr[i - 1] = '\0'; // Remove the trailing newline.
			
			if (equals)
			{
				readptr[equals] = '\0';
				if (!callback(inisection, newsection, readptr, readptr + equals + 1)) // If the callback is false, exit.
				{
					success = false;
					break;
				}
				newsection = false;
			}
			else // If there's no equals, crash with error.
			{
				success = false;
				break;
			}
		}
	}
	
	if (inisection[0])
		free(inisection);
	
	fclose(fp);
	return success;
}
