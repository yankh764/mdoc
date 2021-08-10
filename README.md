# Mdoc - A command-line tool for managing your documents
This program is dedicated to all the command-line lovers that also has lots of documents or  
files to handle and read.  
Mdoc is an improved version of my [Bash Script](https://github.com/yankh764/rdoc) written in C.  
I started devoloping it just out of boredom and of course because I love writing code.


## Main Features
* Counting files
* Listing files
* Printing details and informations on files
* Opening files using your favorite applications
* Sorting results alphabetically
* Colorful output
  
And a lot more...


## Dependencies
* A C Compiler (GCC/Clang)
* GNU Make
* Git


## Installation
1. Cloning the repository:
    ```Bash
    USER # git clone https://github.com/yankh764/mdoc.git
    ```
2. Compiling the code:
    ```Bash
    USER # cd mdoc
    USER # make
    
    Note: by default GCC is used to compile the program but you can change that  
    by typing:

    USER # make CC=clang
    ```
3. Installing the executable to your **PATH**:
    ```Bash
    ROOT # make install
    ```
4. Cleaning build dependencies (optional):
    ```Bash
    USER # make clean
    ```

## Uninstallation
* Go to the cloned directory and type:
    ```Bash
    ROOT # make uninstall
    ```

## Usage
* It's very recommended to read the help message before using the program. You can do that by typing:
    ```Bash
    USER # mdoc
    ```
* You must generate configurations file by typing:
    ```Bash
    USER # mdoc -g
    ```
Note: you can also do that manually. See [Manual Configurations](#manual-configurations).
  
After doing the previous steps you will be ready to enjoy your new document manager :)


## Manual Configurations
* Create a file with the following path: **~/.config/mdoc**
* The configurations file is devided to 3 lines: 
    1. The documents directories paths.
    2. The documents viewer's name.
    3. The additional option and arguments for the documents viewer execution command (optional).
  
After doing the previous steps you will be ready to enjoy your new document manager :)
