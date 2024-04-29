# Coursework
## Requirements
1. Same as for [code2vec](https://github.com/tech-srl/code2vec?tab=readme-ov-file#requirements), excluding ```JavaJDK``` 
2. ```gdown```
3. Any web browser to display ```html``` 

## Build
``` bash
chmod +x build.sh
./build.sh
```

## Download pretrained model (2 Gb)
``` bash
gdown "https://drive.google.com/uc?id=1hBcqrhF3d7LsY36mhSGyY9oY1zEUTHJ6&export=download"
unzip c250k_model.zip
```
or right [here](https://drive.google.com/uc?id=1hBcqrhF3d7LsY36mhSGyY9oY1zEUTHJ6&export=download).

## Run prediction
1. Set parameters in ```run.sh```, particularly:  
    - ```PROBLEM_NAME```  (default = "bit-reverse")
        - problem from examples folder, e.g. "bit-reverse" 
    - ```FILENAME```  (default = "1")
    - ```EXTENSION``` (default = "c")
        - exact solution in PROBLEM_NAME folder 
    - ```MODEL``` (default = "c250k_model/c250k_model")
        - code2vec pretrained model
    - ```TOP_K``` (default = 10) 
        - maximum number of clusters to highlight = number of buttons in generated html    

    and others  

2. Run
    ``` bash
    chmod +x run.sh
    ./run.sh
    ```
3. Run ```data/PROBLEM_NAME/FILENAME/FILENAME.html```, for example
    ``` bash
    firefox data/bit-reverse/1/1.html
    ```
## Directory tree
``` bash
├── build # binaries and makefile data
├── c250k_model # folder with downloaded pretrained model
├── data # predictions
│  ├──  PROBLEM_NAME
│  │  ├── FILENAME
│  │  │  ├── FILENAME.contexts # extractor job
│  │  │  ├── FILENAME.contexts.num_examples # by default all C_n^2 contexts
│  │  │  ├── FILENAME.contexts.vectors # code2vec job
│  │  │  ├── FILENAME.contexts.vectors.clusters # clustering job
│  │  │  └── FILENAME.html # result file to visualize
│  │  └── ...
│  └── ...
├── examples # use to evaluate model
│  ├──  PROBLEM_NAME
│  │  ├── test # folder with files
│  │  │  ├── FILENAME.EXTENSION 
│  │  │  └── ...
│  │  ├── clusters.sav # trained kmeans model
│  │  ├── solutions_pt.csv # info about pt clusters
│  │  └── statement.md # problem statement
│  └── ...
├── extractor # tool to extract contexts from files
│  ├── CMakeLists.txt
│  ├── extractor.h # base classes to extract contexts
│  ├── main.cpp
│  ├── threadPool.h # some parallel stuff
│  └── treeSitter.h # C++ binding on tree-sitter
├── frontend
│  ├──  ast # tool to draw syntax tree
│  │  ├── CMakeLists.txt
│  │  └── main.cpp
│  └── error_detection # tool to detect "error" clusters, generates html
│     ├── CMakeLists.txt
│     ├── highlight.h # several classes for html generation
│     ├── main.cpp
│     └── predict.py # generates csv files for visualizer
├── model # code2vec model files
├── tree-sitter # submodule
├── tree-sitter-c # submodule
├── .clang-format
├── build.sh # script to configure cmake
├── CMakeLists.txt
├── README.md
└── run.sh # script to predict error clusters
```

