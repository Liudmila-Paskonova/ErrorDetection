#!/usr/bin/env bash
###########################################################
# Script to predict error classes for test file
# Change the following values to generate html highlighting maybe-error contexts.
#   PROBLEM_NAME is a problem from examples folder, e.g. "bit-reverse"
#   FILENAME with EXTENSION is an exact solution in PROBLEM_NAME folder
#   MAX_CONTEXTS is the number of contexts to keep for each 
#     method (by default 300).   
#   WORD_VOCAB_SIZE, PATH_VOCAB_SIZE, TARGET_VOCAB_SIZE -   
#     - the number of words, paths and target words to keep 
#     in the vocabulary (the top occurring words and paths will be kept). 
#     The default values corresponds to values used to train base model c250k
#   MAX_PATH - the maximum length of the context 
#     (e. g. max length of the path between two terminals, number of nodes)
#   MAX_WIDTH - the maximum number of children each internal node may have
#   PYTHON - python3 interpreter alias.
#   MODEL - code2vec pretrained model
#   DEFAULT_EMBEDDINGS_SIZE - d parameter, length of code vector
#   TOP_K - maximum number of clusters to highlight 
#     (e.g. number of buttons in generated html)
 
PROBLEM_NAME=bit-reverse
FILENAME=1
EXTENSION=c
MAX_CONTEXTS=300
WORD_VOCAB_SIZE=22394
PATH_VOCAB_SIZE=1300001
TARGET_VOCAB_SIZE=300
MAX_PATH=12
MAX_WIDTH=5
PYTHON=python3
MODEL=c250k_model/c250k_model
DEFAULT_EMBEDDINGS_SIZE=128
TOP_K=10
###########################################################
TEST_FILE_FOLDER=data/${PROBLEM_NAME}/${FILENAME}
TEST_DATA_FILE=${TEST_FILE_FOLDER}/${FILENAME}.raw.test.txt
TEST_FILE=examples/${PROBLEM_NAME}/test/${FILENAME}.${EXTENSION}
CLUSTERING_MODEL=examples/${PROBLEM_NAME}/clusters.sav
SOLUTIONS_PT=examples/${PROBLEM_NAME}/solutions_pt.csv

mkdir -p data
mkdir -p data/${PROBLEM_NAME}
mkdir -p ${TEST_FILE_FOLDER}

# EXTRACTOR
# extract contexts from example file
./build/bin/extractor --predict ${TEST_FILE} --max_path_length ${MAX_PATH} --max_path_width ${MAX_WIDTH} > ${TEST_DATA_FILE} 

TARGET_HISTOGRAM_FILE=${TEST_FILE_FOLDER}/${FILENAME}.histo.tgt.c2v
ORIGIN_HISTOGRAM_FILE=${TEST_FILE_FOLDER}/${FILENAME}.histo.ori.c2v
PATH_HISTOGRAM_FILE=${TEST_FILE_FOLDER}/${FILENAME}.histo.path.c2v

cat ${TEST_DATA_FILE} | cut -d' ' -f1 | awk '{n[$0]++} END {for (i in n) print i,n[i]}' > ${TARGET_HISTOGRAM_FILE}
cat ${TEST_DATA_FILE} | cut -d' ' -f2- | tr ' ' '\n' | cut -d',' -f1,3 | tr ',' '\n' | awk '{n[$0]++} END {for (i in n) print i,n[i]}' > ${ORIGIN_HISTOGRAM_FILE}
cat ${TEST_DATA_FILE} | cut -d' ' -f2- | tr ' ' '\n' | cut -d',' -f2 | awk '{n[$0]++} END {for (i in n) print i,n[i]}' > ${PATH_HISTOGRAM_FILE}

${PYTHON} model/preprocess.py --train_data ${TEST_DATA_FILE} --max_contexts ${MAX_CONTEXTS} --word_vocab_size ${WORD_VOCAB_SIZE} --path_vocab_size ${PATH_VOCAB_SIZE} \
--target_vocab_size ${TARGET_VOCAB_SIZE} --word_histogram ${ORIGIN_HISTOGRAM_FILE} --path_histogram ${PATH_HISTOGRAM_FILE} --target_histogram ${TARGET_HISTOGRAM_FILE} \
--output_name ${TEST_FILE_FOLDER}/${FILENAME}
    
rm ${TEST_DATA_FILE} ${TARGET_HISTOGRAM_FILE} ${ORIGIN_HISTOGRAM_FILE} ${PATH_HISTOGRAM_FILE}
TEST_CONTEXTS=${TEST_FILE_FOLDER}/${FILENAME}.contexts

# VECTORS
# - get contexts' vector representation
TEST_VECTORS=${TEST_CONTEXTS}.vectors
str="TOKEN1_START_BYTE,TOKEN1_END_BYTE,TOKEN2_START_BYTE,TOKEN2_END_BYTE"
for (( k=0; k<3*${DEFAULT_EMBEDDINGS_SIZE}; k++ ))
do
str="${str},${k}"
done
echo "$str" > ${TEST_VECTORS}

${PYTHON} model/code2vec.py --load ${MODEL} --test ${TEST_CONTEXTS} --export_code_vectors
rm log.txt

# VISUALIZER
# - get html
TEST_FILE_HTML=${TEST_FILE_FOLDER}/${FILENAME}.html
${PYTHON} frontend/error_detection/predict.py --vectors ${TEST_VECTORS} --model ${CLUSTERING_MODEL} --solutions_pt ${SOLUTIONS_PT} -top_k ${TOP_K}
./build/bin/errors ${TEST_FILE} test.txt > ${TEST_FILE_HTML}

rm test.txt

# AST
# - get png
./build/bin/tree ${TEST_FILE} test.dot
dot test.dot -T png -o ${TEST_FILE_FOLDER}/${FILENAME}.png
rm test.dot