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
```Bash
USER # cd mdoc
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
You can also do that manually. See [Manual Configurations](#manual-configurations).

## Manual Configurations
