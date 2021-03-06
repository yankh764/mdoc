# Mdoc - A command-line tool for managing your documents
Mdoc is an improved version of my [Bash Script](https://github.com/yankh764/rdoc) written in C.  
Just as it's description says, it's a command-line tool for managing your documents and easing your life.  
I started developing it just out of boredom and of course because I love writing code.
  
This program is dedicated to all the command-line lovers that also has lots of documents or  
files to read and handle.


## Main Features
* Counting files
* Listing files
* Printing details and informations on files
* Opening files using your favorite applications
* Sorting results alphabetically
* Colorful output
  
And a lot more... See [Mdoc Help Message](#mdoc-help-message)


## Dependencies
* A C Compiler (GCC/Clang)
* GNU Make
* Git


## Installation
1. Cloning the repository:
    ```
    $ git clone https://github.com/yankh764/mdoc.git
    ```
2. Compiling the code:
    ```
    $ cd mdoc
    $ make
    
    Note: by default GCC is used to compile the program but you can change that  
    by typing:

    $ make CC=clang
    ```
3. Installing the executable to your **PATH**:
    ```
    # make install
    ```
4. Cleaning build dependencies (optional):
    ```
    $ make clean
    ```

## Uninstallation
* Go to the cloned directory and type:
    ```
    # make uninstall
    ```

## Usage
* It's highly recommended to read the [help message](#mdoc-help-message) before using the program. You can do that by typing:
    ```
    $ mdoc
    ```
* You must generate configurations file by typing:
    ```
    $ mdoc -g
    ```
Note: you can also do that manually. See [Manual Configurations](#manual-configurations).
  
After doing the previous steps you will be ready to enjoy your new document manager :)


## Manual Configurations
* Create a file with the following path: `~/.config/mdoc`
* The configurations file is divided to 3 lines: 
    1. The documents directories paths.
    2. The documents viewer's name.
    3. The additional option and arguments for the documents viewer execution command (optional).
  
After doing the previous steps you will be ready to enjoy your new document manager :)


## Mdoc Help Message
* I decided to include it here so you could take a look at it, see all of it's features and decide if it'll serve you:
    ```
    Usage: mdoc [OPTIONS]... ARGUMENT
    A command-line tool for managing your documents and easing your life.

    Available options:
     -h 		 Display this help message
     -g 		 Generate new configurations file
     -s 		 Sort the founded documents alphabetically
     -r 		 Reverse the order of the founded documents
     -a 		 Include all documents
     -i 		 Ignore case distinctions while searching for the documents
     -n 		 Allow numerous documents opening (execution)
     -c 		 Count the existing documents with the passed string sequence in their names
     -l 		 List the existing documents with the passed string sequence in their names
     -d 		 Display details on the documents with the passed string sequence in their names
     -o 		 Open the founded document with the passed string sequence in it's name
     -R 		 Disable recursive searching for the documents
     -C 		 Disable colorful output


    NOTES:
      1. It's good to note that the program has multiple directories support when
         searching for a document. So when generating the configurations you can pass
         more than one directory absolute path which the program will search for
         documents in it at a run time. Please separate the paths with a space.
         Example: /path/to/dir1 /path/to/dir2 /path/to/dir3...

      2. When generating the configurations, if it's desired to pass additional
         arguments for the documents execution command, please separate them with
         a space. Example: --arg1 --arg2 --arg3...

      3. By default when using the -o option you can't open more than a document
         in a run, but you can use the -n option with it to give the program the
         approval to open more than one document in a run.

      4. You can use the -a optoin with the -c, -l, -d and -o options instead
         of passing an actual argument.

      5. The [TIME] section in the -d option stands for the last modification
         time, or if the document haven't been modified once, it'll stand for
         the creation time of the document.


    EXIT CODES:
     0   Success
     1   Error in the command line syntax
     2   General error in the program
    ```


## TODO
* Adding support for long options.


## Contributing
Pull requests are welcomed...

