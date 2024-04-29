import numpy as np
import pandas as pd
from argparse import ArgumentParser
import pickle

if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("-v", "--vectors", dest="vectors",
                        help="vectors of contexts", required=True)
    parser.add_argument("-mod", "--model", dest="model",
                        help="path to file with clustering model", required=True)
    parser.add_argument("-pt", "--solutions_pt", dest="solutions_pt",
                        help="path to file with onfo about pt solutions", required=True)
    parser.add_argument("-top_k", "--top_k_clusters", dest="top_k",
                        help="top k clusters to choose errors", required=True)

    args = parser.parse_args()

TEST_FILE = args.vectors
MODEL_FILE = args.model
SOLUTIONS_PT_FILE = args.solutions_pt
TOP_K = np.int32(args.top_k)

pt_solutions = np.loadtxt(SOLUTIONS_PT_FILE)
df = pd.read_csv(TEST_FILE)
model = pickle.load(open(MODEL_FILE, "rb"))
X_clusters = model.predict(df.iloc[:, 4:])
pt_clusters_all = model.labels_.copy().astype(np.int64)

info = pd.DataFrame()
info.insert(0, "CLUSTER", pt_clusters_all, True)
info.insert(1, "WEIGHT", pt_solutions, True)
info = info.groupby('CLUSTER')['WEIGHT'].max().reset_index()
info = info.sort_values(by=["WEIGHT"], ascending=False)
# info.to_csv(TEST_FILE + ".info", index=False)

clusters = df.iloc[:, 0:4]
clusters.insert(4, "CLUSTER", X_clusters, True)
weight_values = []
for i in X_clusters:
    weight_values.append(info.loc[info['CLUSTER'] == i, 'WEIGHT'])

clusters.insert(5, "WEIGHT", np.ravel(np.array(weight_values)), True)
clusters = clusters.sort_values(by=["WEIGHT"], ascending=False)
clusters.to_csv(TEST_FILE + ".clusters", index=False)
uniq = np.array(clusters["CLUSTER"].drop_duplicates())
uniq = uniq[0:TOP_K]

res = clusters.loc[clusters["CLUSTER"].isin(uniq), [
    "TOKEN1_START_BYTE", "TOKEN1_END_BYTE", "TOKEN2_START_BYTE", "TOKEN2_END_BYTE", "CLUSTER"]]

res.to_csv("test.txt", index=False, header=False, sep=' ')
