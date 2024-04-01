# my toolkit: 
## fork():
	make children processes
	keep track of exit status with wait, EIFEXITED, WEXITSTATUS
	can launch 3 processes for one child
	can make each child run a process
	can launch proceses for only children
	examples: 
		checkout week_7_tut -> fork2.c
			 week_8_tut -> childcreates.c parentcreates.c

# pipe():
	i can write to pipe
	can use this to communicate between parent and child
	can have pipes for each child
	e.g. fd[num_kids][2]
	examples: 
		checkout pls fix myPipes2.c from class
			 show_pipe.c
			 week_9_tut -> checkpassword.c
			 week_9_tut -> validate.c
			 week_8_tut -> pipes.c

## signals():
	can send signals with signal handler...
	can call function
	can percept signals from input
	examples:
		checkout 'week_9_tut -> signals.c'

when terminating early, make sure u free all the memory correctly! 
no zombies, orphans!
figure out signals ...
