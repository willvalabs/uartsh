/*
 * Author: Chandra Mohan C (willvalab@gmail.com)
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file.
 *
**-----------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "./uartsh.h"
/*-----------------------------------------------*/

size_t uartsh_puts(char* buffer, size_t size)
{
	int count = size;
	while(count--)
	{
		UARTSH_CONFIG_uart_putc(*buffer++);
	}

	return size;
}
/*-----------------------------------------------*/

size_t uartsh_gets(char buffer[], size_t size)
{
	size_t cCount = 0;
	char c = 0;

	if( size > UARTSH_CONFIG_COMMAND_STRING_SIZE )
		size = UARTSH_CONFIG_COMMAND_STRING_SIZE;

	// reserve space for \n and \0
	size -= 2;

	while( cCount < size )
	{
		c = (char) UARTSH_CONFIG_uart_getc();

		if( (c == '\r')  || (c == '\n') )
			break;

		//only alphabets and special characters allowed
		if( (c > 31) && (c < 127) )
		{
			UARTSH_CONFIG_uart_putc(c);
			buffer[cCount++] = c;
		}
		else if( c == '\b' )
		{
			if( cCount )
			{
				UARTSH_CONFIG_uart_putc('\b');
				UARTSH_CONFIG_uart_putc(' ');
				UARTSH_CONFIG_uart_putc('\b');
				cCount--;
			}
		}
	}

	UARTSH_CONFIG_uart_putc('\n');
	buffer[cCount++] = '\n';
	buffer[cCount] = '\0';

	return cCount;
}
/*-----------------------------------------------*/

int uartshOpen( const UartshCommand commands[] )
{
	char buffer[UARTSH_CONFIG_COMMAND_STRING_SIZE] = { 0, };
	char* argv[UARTSH_CONFIG_ARGC_MAX + 1] = { 0, };
	int argc = 0;

	while(1)
	{
		printf("\n"UARTSH_CONFIG_PROMPT_STRING" ");
		fflush(UARTSH_CONFIG_STDOUT);

#if UARTSH_USE_NEWLIB_FGETS
		fgets( buffer, sizeof(buffer), UARTSH_CONFIG_STDIN );
#else
		if( 0 == uartsh_gets(buffer, sizeof(buffer)) )
			continue;
#endif

		argc = 0;
		char* token = strtok(buffer, " \n");
		while(token != NULL)
		{
			argv[argc++] = token;
			token = strtok(NULL, " \n");
		}

		if( !argc )
			continue;

		UARTSH_CONFIG_uart_putc('\n');

		size_t commandLength = strlen(argv[0]);
		if( 1 == argc )
		{
			if( (commandLength == 2) && (0 == strncmp("ls", argv[0], 2)) )
			{
				puts("Supported commands are below:\n");
				int index = 0;
				while( commands[index].name != NULL )
				{
					printf("  %s\n", commands[index++].name);
				}

				continue;
			}
			else if( (commandLength == 4) && (0 == strncmp("exit", argv[0], 4)) )
				return 0;
		}

		const UartshCommand* pUserApp = NULL;
		int index = 0;
		while( commands[index].name != NULL )
		{
			size_t appNameSize = strlen(commands[index].name);

			// extra check to ensure command length is exactly matching app length
			if( commandLength == appNameSize )
			{
				if( 0 == strncmp(argv[0], commands[index].name, appNameSize) )
				{
					pUserApp = &commands[index];
					break;
				}
			}

			index++;
		}

		if( NULL != pUserApp )
			pUserApp->handler(argc, argv);
		else
			puts("unknown command");
	}

	return 0;
}
/*-----------------------------------------------*/

int uartshParseCommand(UartshCommandParser* parser, int argc, char* argv[])
{
    struct argparse argparse;
    argparse_init(&argparse, parser->options, parser->usages, 0);
    argparse_describe(&argparse, parser->description, NULL);
	return argparse_parse(&argparse, argc, argv);
}
/*-----------------------------------------------*/