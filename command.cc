
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand = Command::_currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    // If the previous token is expecting a file in the next token
    TokenType expecting = TOKEN_EOF;

    for (int i = 0; i < tokens.size(); i++) {
        // create new simple command whenever a pipe is encountered
        // append arguments to current simple command
        if (expecting != TOKEN_EOF) {
            // If the token after is not as expected, then an error is output
            if (tokens[i].type != TOKEN_COMMAND && tokens[i].type != TOKEN_ARGUMENT) {
                fprintf(stderr, "myshell: syntax error: expected filename after redirect\n");
                delete currentSimpleCommand;
                currentCommand.clear();
                return;
            }

            // command parameters are adjusted based on previous token

            // std::string.data() is used to set attribute for command object as a mutable char*
            char* file = strdup(tokens[i].value.c_str());

            switch (expecting) {
                case TOKEN_REDIRECT:
                    currentCommand._outFile = file;
                    break;
                case TOKEN_APPEND:
                    currentCommand._outFile = file;
                    currentCommand._append = 1;
                    break;
                case TOKEN_INPUT:
                    currentCommand._inputFile = file;
                    break;
                case TOKEN_ERROR:
                    currentCommand._errFile = file;
                    break;
                case TOKEN_REDIRECT_AND_ERROR:
                    currentCommand._outFile = file;
                    currentCommand._out_error = 1;
                    currentCommand._append = 1;
                    break;              
                default:
                    break;
            }

            expecting = TOKEN_EOF;
            continue;
        }

        // Any other token found
        switch (tokens[i].type) {
            case TOKEN_COMMAND:
            case TOKEN_ARGUMENT:
                currentSimpleCommand->insertArgument(strdup(tokens[i].value.c_str()));
                break;
            case TOKEN_BACKGROUND:
                // If there are no commands before the &
                if (currentSimpleCommand->_numberOfArguments == 0) {
                    fprintf(stderr, "myshell: syntax error: empty command before '&'\n");
                    delete currentSimpleCommand;
                    currentCommand.clear();
                    return;
                }
                currentCommand.insertSimpleCommand(currentSimpleCommand);
                currentSimpleCommand = new SimpleCommand();
                currentCommand._background = 1;     // set background in command attribute
                break;
            case TOKEN_PIPE:
                // If there are no commands before the |
                if (currentSimpleCommand->_numberOfArguments == 0) {
                    fprintf(stderr, "myshell: syntax error: empty command before '|'\n");
                    delete currentSimpleCommand;
                    currentCommand.clear();
                    return;
                }
                currentCommand.insertSimpleCommand(currentSimpleCommand);
                currentSimpleCommand = new SimpleCommand();
                break;

            // Send to expecting switch for command attribute setting
            case TOKEN_REDIRECT:
            case TOKEN_APPEND:
            case TOKEN_INPUT:
            case TOKEN_ERROR:
            case TOKEN_REDIRECT_AND_ERROR:
                expecting = tokens[i].type;
                break;

            case TOKEN_EOF:
                // Add any remaining arguments in the current simple command into the current command
                if (currentSimpleCommand->_numberOfArguments > 0) {
                    currentCommand.insertSimpleCommand(currentSimpleCommand);
                    // clear pointer
                    currentSimpleCommand = nullptr;
                } else {
                    // If it is empty, free any memory
                    delete currentSimpleCommand;
                    currentSimpleCommand = nullptr;
                }

                // If no simple commands were parsed at all, clear and exit
                if (currentCommand._numberOfSimpleCommands == 0) {
                    currentCommand.clear();
                    return;
                }

                // Run command after all tokens have been parsed
                currentCommand.execute();
                return;

            default:
                fprintf(stderr, "myshell: unknown token type %d at token \"%s\"\n", (int) tokens[i].type, tokens[i].value.c_str());
                break;
        }
    }
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }
    // free(_simpleCommands);
    // _simpleCommands = NULL;
    // _numberOfAvailableSimpleCommands = 1;

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    // No execution if there are no commands
    if (_numberOfSimpleCommands == 0) {
        clear();
        prompt();
        return;
    }

    /*
        Notes:
        - To use dup2, reference: https://www.geeksforgeeks.org/c/dup-dup2-linux-system-call/
            int fd = open(file, flags);
            dup2(fd, STDIN_FILENO | STDOUT_FILENO | STDERR_FILENO);
            close(fd);

          Open the original file, duplicate its descriptor, then close the original descriptor
          Then any print statements after the dup2 will output to the file descriptor, or take input from file descriptor
    */

    // Indicates if there was any pipe feeding into current one
    int prev = -1;

    // Reserve space to store all PIDs of children processes
    std::vector<pid_t> pids;
    pids.reserve(_numberOfSimpleCommands);

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        // Determines if this process is the last one in the pipeline
        bool last = (i == _numberOfSimpleCommands - 1);
    
        // Create a pipe HERE
    
        // Creating child process
        pid_t pid = fork();
        
        if (pid < 0) {
            // ERROR
            
        }

        if (pid == 0) {
            // CHILD

            // If there was a pipe before this one, redirect its output as input to this one
            if (prev != -1) {
                // Close child process if any error occurs
                if (dup2(prev, STDIN_FILENO) < 0) {
                    perror("dup2 stdin");
                    _exit(1);
                }
                // Close previous pipe
                close(prev);
            }

            if (_inputFile) {

            }

            if (_outFile) {
                
            }

            if (_errFile) {

            }

            if (_out_error) {

            }

            // Execute command
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);

            // Only output error if execute fails, then exit
            perror(_simpleCommands[i]->_arguments[0]);
            _exit(1);
        }

        if (pid > 0) {
            // PARENT
            pids.push_back(pid);
            if (prev != -1) close(prev);
        }
    }

    // Wait for child processes to finish first
    // if TOKEN_BACKGROUND used, it will not wait
    if (!_background) {
        for (pid_t p : pids) {
            int status;
            waitpid(p, &status, 0);
        }
    }

    clear();
    prompt();
}

void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true) 
    {
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
