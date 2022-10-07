<div id="top"></div>

<br />
<div align="center">
  <h1 align="center">Parallel Huffman file compression</h1>
  <h3 align="center">Aristotle University of Thessaloniki</h3>
  <h4 align="center">School of Electrical & Computer Engineering</h4>
  <p align="center">
    Contributors: Kyriafinis Vasilis
    <br />
    Winter Semester 2022 - 2023
    <br />
    <br />
  </p>
</div>


<!-- TABLE OF CONTENTS -->
- [1. About This Project](#1-about-this-project)
- [2. Getting Started](#2-getting-started)
- [3. Dependencies](#3-dependencies)
    - [3.1. Make](#31-make)
    - [3.2. OpenCilk](#32-opencilk)
    - [3.3. uint256_t library](#33-uint256_t-library)
- [4. Usage](#4-usage)

## 1. About This Project

The objective of this project is to compare the the pthreads and openCilk multithreading systems. The project is based on the implementation of a parallel Huffman file compression algorithm. The main points of comparison are the functionalities, the performance and the ease of use of the two multithreading systems. The project is implemented in C++ but the logic of the algorithm is based on the C language. The C++ language was used for compatibility reasons with the uint256_t library (see [dependencies](https://github.com/Billkyriaf/pds_assignment_4#dependencies)).

## 2. Getting Started

To setup this repository on your local machine run the following command on the terminal:

```console
$ git clone git@github.com:Billkyriaf/pds_assignment_4.git
```

Or alternatively [*download*](https://github.com/Billkyriaf/pds_assignment_4/archive/refs/heads/main.zip) and extract the zip file of the repository.

## 3. Dependencies
#### 3.1. Make

This project uses make utilities to build and run the executables.

#### 3.2. OpenCilk

You can install OpenCilk by following the instructions of the official [website](https://www.opencilk.org/doc/users-guide/install/#installing-using-a-tarball). The official support is for Ubuntu but the binaries have been tested on Manjaro Linux as well and they are functional.

#### 3.3. uint256_t library

The uint256_t library is used to handle the 256 bit integers used in the project. The library is included in the repository and it is not necessary to install it. If you want to find out more about it check the official [repository](https://github.com/calccrypto/uint256_t)

## 4. Usage

To build the executables run the following command on the terminal:

```console
$ make 
```