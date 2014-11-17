-------
ReadMe
-------

Contents
--------

1. GenSig - generates random bit vectors

2. KMeansSig - clusters bit vectors


---------------
1 . GenSig.exe
---------------

GenSig generates random bit vectors and writes them to disk.

The sparsity/density of the bit vectors may be controlled by changing
the parameter p of the Bernoulli distribution which is used to
generate 0's and 1's.

The file format is:

line 1. <int> (dimension of vector)
line 2. bytes (binary data for all vectors)


Parameters
----------
dim : (required) : the dimension of the vectors

out : (required) : the name of the file to write the vectors to

numvecs : (required) : the number of vectors to generate

p : (optional) : default = 0.5 : the parameter of the Bernoulli distribution

seed : (optional) : the seed for the random number generator


Examples
--------




-----------------
2. KMeansSig.exe
-----------------

KMeansSig.exe performs K-means clustering on bit valued vectors (also called signatures).

The distance measure is Hamming distance.

The quality of the solution is quantified using average RMSE in bits. 

A "simulated annealing" method has been added which can improve the final solution,
usually by only a small number of bits.


Parameters
----------

in : (required) : the name of the file containing the vectors to cluster. The
  format of the file may be one of two types:
  1) vector data in bytes.
  2) line 1: the vector dimension, line 2: the data in bytes for all vectors.
  See the "dim" parameter below.

clusters : (required) : the number of clusters

ids : (optional) : the name of the file containing string identifiers for
  the vectors to cluster. One id is given per line of the file. If this value
  is not given then a default incrementing numeric identifier will be given
  to each vector in the order in which the vectors appear in the input file.

out : (optional) : the name of the file to write the solution to.
  If this value is not given then the solution will not be written to disk.
  
dim : (optional/required) : the dimension of the vectors.
  If the vector dimension is not specified on the command line it is assumed
  that this value is contained in the first line of the input file. 
  If this value is given on the command line then it will be assumed that it
  is not contained in the input file and that the first byte of the input file
  is the first byte of the vector data.

threads : (optional) : default = 1 : the number of threads to use.
  Suggested value is 2 x num_of_cores. 

maxvecs : (optional) : the maximum number of vectors to read from the input file.

maxiters : (optional) : default = 20 : the maximum number of k-means iterations
  to use.

eps: (optional) : default = 0 : the difference in average RMSE between
  consecutive iterations at which the clustering should be terminated. If this
  value is 0 (the default) then the clustering will terminate when either maxiters
  is reached or if there is no change in cluster membership.
  
saiters : (optional) : default = 0 : the number of iterations of simulated annealing
  to perform.

sastart: (optional) : a probability value used for simulated annealing. This value
  determines the likelihood that a vector will be randomly allocated to another
  cluster during the annealing process. This value is gradually decreased over a
  number of iterations given by <saiters>.
  

Examples
--------

example 1:

KMeansSig -in "C:/Data/wikisignatures/wiki.4096.sig" -clusters 200 -threads 16 -out out


example 2:

KMeansSig -in "C:/Data/wikisignatures/wiki.4096.sig" -clusters 200 -threads 16 -out out
-maxIters 10 -saiters 10 -sastart 0.2







