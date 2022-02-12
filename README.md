
A basic Unix shell capable of looping commands, redirecting output, and processing batch commands fed through .txt files. Try it in your local Linux terminal!

### Usage

Enter  ```./shell ``` in your terminal and start using it like a normal shell. Alternatively, if you have a file with a batch of commands, enter ```./shell commands.txt```  to run all of the commands in the file at once. Try this with the batch.txt provided

You can run most commands you normally run in bash. There are two additional
commands:

1. ```path <search/path1> <search/path2> ...```: This command adds ```<search/path1>``` and ```<search/path2>``` ... to the search path of the shell. This means that the paths entered should contain the executables for commands the users wish to carry out, since the shell will only scan through these paths to find the commands. If it is set to empty, only '''cd''', '''path''', and '''exit''' will work.


2. ```loop <n> <command> ...```: The loop command runs the specified command n times. ```loop <n> <command> ... $loop```: A variation of the ```loop``` command which prints out, when executing the commands, how many times the command has been run.

To exit, simply use the ```exit``` command.
