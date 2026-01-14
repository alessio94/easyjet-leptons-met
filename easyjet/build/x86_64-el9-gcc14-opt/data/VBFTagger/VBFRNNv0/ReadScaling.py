import pickle

f = open('scaling', "rb")
varinfo = pickle.load(f)

print(varinfo)
