# Unix-Shell
A command line interpreter (CLI) or, as it is more commonly known, a shell, like, but simpler than, the one you run in Unix.
The shell operate in this basic way: when you type in a command (in response to its prompt), the shell creates a child process that executes the command you entered and then prompts for more user input when it has finished.  

This basic shell, called *dash*, is basically an interactive loop: it repeatedly prints a prompt dash> (note the space after the greater-than sign), parses the input, executes the command specified on that line of input, and waits for the command to finish. This is repeated until the user types *exit*. The name of your final executable should be dash.  

The shell can be invoked with either no arguments or a single argument; anything else is an error. Here is the no-argument way:
```bash
prompt> ./dash
dash>
```

At this point, dash is running, and ready to accept commands. Type away!
The mode above is called interactive mode. It allows the user to type commands directly. The shell also supports a batch mode, which instead reads input from a batch file and executes commands from therein. Here is how you run the shell with a batch file named *batch.txt*:
```bash
prompt> ./dash batch.txt
```

One difference between batch and interactive modes: in interactive mode, a prompt is printed (dash>). In batch mode, no prompt should be printed.
This basic shell is able to parse a command and run the program corresponding to the command. For example, if the user types *ls -la /tmp*, this shell will run the program */bin/ls* with the given arguments *-la* and */tmp*.

## Some demo testcases:
### interactive mode:
```bash
prompt> ./dash
```
![image](https://github.com/JesseLee62/img-storage/blob/master/Unix-Shell/interactive-sample.jpg)
![image](https://github.com/JesseLee62/img-storage/blob/master/Unix-Shell/interactive-sample2.jpg)
****
### batch mode:
##### t1.txt:   
*echo hello world  
sleep 4  
ls & ps -u  
cat output.txt  
echo hi > output.ext*

##### output.txt:  
*echo This is output.txt*

```bash
prompt> ./dash t1.txt
```
![image](https://github.com/JesseLee62/img-storage/blob/master/Unix-Shell/batchmode.jpg)

