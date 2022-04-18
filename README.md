# The CheckCBox clang repo

This repo contains a version of the LLVM/Clang toolchain that is being modified
to support the CheckCBox fork of Checked C. Checked C extends
C with checking to detect or prevent common programming errors such as out-of-bounds memory accesses.
The SSDP fork of the Checked
C specification is available at the
[SSDP Checked C repo](https://github.com/secure-sw-dev/checkedc).

## Trying out CheckCBox

Programmers are welcome to use CheckCBox as it is being implemented.  You will
have to build your own copy of the compiler. 
Below are the directions on how to do it.
```
cd <WORK_DIR>
```
```
git clone https://github.com/arunkumarbhattar/CheckCBox_Compiler.git src
```

The Checked C language tests live in a folder within llvm/project. Change to the src/llvm/projects/checkedc-wrapper directory and clone the Checked C repo.

```
git clone https://github.com/Microsoft/checkedc
```

Now change to src/llvm/projects/checkcbox-wrapper directory and clone the checkcbox repo

```
git clone https://github.com/arunkumarbhattar/checkcbox.git
```
Now create a build directory generate the Cmake files 

```
cd <WORK_DIR>/build
cmake -G Ninja -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_LINKER=/usr/bin/gold DCMAKE_BUILD_TYPE=Debug -DLLVM_LIT_ARGS=-v -DLLVM_PARALLEL_LINK_JOBS=5 <WORK_DIR>/src/llvm
```
Now you are all set to build the target 
Execute any of the following command in the build directory 
```
ninja // this command will build the compiler and all other supporting tools
```
```
ninja clang //this command will only build the compiler
```
```
ninja check-checkcbox // This command will run all the sanity test cases for checkcbox project 
```
```
ninja check-checkedc // This command will run all the sanity test cases for checked project 
```
```
ninja check-clang // This command will run all the sanity test cases for clang frontend
```
## Testing Playground for CheckCBox

After the checkcbox tests have been cloned in the src/llvm/projects/checkcbox-wrapper directory by performing git clone https://github.com/arunkumarbhattar/checkcbox.git in the above mentioned directory

You are presented with a comprehensive test case suit that test various aspets of checkcbox.

Please switch to https://github.com/arunkumarbhattar/checkcbox/tree/main/tests directory in your respective project folder.

From here you can switch to directory of any particular sub-feature you are interested in and add/modify code to test the operation of your interest.
After the file has been changed, you can directory compile the target "check-checkcbox" in your build directory to test the changes.
