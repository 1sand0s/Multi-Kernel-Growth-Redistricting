# Multi-Kernel-Growth-Redistricting
![GitHub](https://img.shields.io/github/license/1sand0s/Multi-Kernel-Growth-Redistricting)

Multi-Kernel Growth strategy based technique for fair redistricting
 

<h2>How to build</h2>

1. <h4>Clone the repo:</h4>

    Clone the repository using `git clone https://github.com/1sand0s/Multi-Kernel-Growth-Redistricting`

2. <h4>Install RapidJson:</h4>

    Clone the `rapidjson` repository using `git clone https://github.com/Tencent/rapidjson` into `Multi-Kernel-Growth-Redistricting` directory.

3. <h4>Install Ncurses:</h4>

    3.1. `git clone https://github.com/mirror/ncurses.git`\
    3.2. `cd ncurses`\
    3.3. `./configure --prefix ~/local --enable-widec --with-pthread`\
    3.4. `make -j`\
    3.5. `make -j install`

4. <h4>Building:</h4>

    `cd` into the `src` directory of `Multi-Kernel-Growth-Redistricting` and run `make` 


<h2>Running</h2>


1. <h4>How to run:</h4>

   run `./main us-county-boundaries.json tx_county_population.csv us-colleges-and-universities.json 36 1`


2. <h4>Argument definitions:</h4>

   2.1. County Borders
   2.2. Population per County
   2.3. Number of universities per County
   2.4. Number of Congressional Districts
   2.5. Gerrymandering on/off

   All data used are obtained from : https://public.opendatasoft.com/explore/dataset/
