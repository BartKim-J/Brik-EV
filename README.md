# **Brik EV Project**

[![build](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://bitbucket.org/xengiennering/sn3d-project)  [![platform](https://img.shields.io/badge/platform-linux-lightgrey.svg)]()
### **Package**
[![cmake](https://img.shields.io/badge/cmake-3.5-brightgreen.svg)](https://cmake.org/) [![gcc](https://img.shields.io/badge/gcc-6.3.0-brightgreen.svg)](https://gcc.gnu.org/) [![gdb](https://img.shields.io/badge/gdb-7.12.0-brightgreen.svg)](https://www.gnu.org/software/gdb/)

### **Library**

# **Features**
---

> This README.md uses the [Markdown](http://daringfireball.net/projects/markdown/) syntax. The [MarkDownDemo tutorial](https://bitbucket.org/tutorials/markdowndemo) shows how various elements are rendered. The [Bitbucketdocumentation](https://confluence.atlassian.com/bitbucket/readme-content-221449772.html) has more information about using a README.md.


# **Install & Compile**
---
#### **_Step 1._** Clone git repository.
```
$ git clone https://bitbucket.org/xengiennering/brik-ev.git
```

#### **_Step 2._** Make makeFile by cmake and check file list.
```
$ cmake CMakeList.txt
```

# **Make Commands**
---
#### Build
`$ make all` - Build project.

`$ make clean` - Clean project. ( not remove cmakeFiles just build file. )

`$ make all_clean` - build file and all cmake file is remove. 

#### Run & Debug
`$ make gdb` - Brik firmware start with GDB. First do `$ make stop`.

`$ make run` - Brik firmware  start without GDB. First do `$ make stop`.

`$ make airplay` - airplay software start.

#### API Document
`$ make doxygen` - 

# **Reference**
---
* ### Software

* ### Hardware

* ### Linux OS


#### TODO List
- [X] NC.
- [ ] NC.

#### BUG List
- [X] NC.
- [ ] NC.
