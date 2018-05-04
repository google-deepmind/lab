# MNIST dataset download instructions
http://yann.lecun.com/exdb/mnist/

The MNIST database of handwritten digits, available from this page, has a
training set of 60,000 examples, and a test set of 10,000 examples. It is a
subset of a larger set available from NIST. The digits have been size-normalized
and centered in a fixed-size image.

```sh
curl 'http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz' > train-images-idx3-ubyte.gz
curl 'http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz' > train-labels-idx1-ubyte.gz
curl 'http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz' > t10k-images-idx3-ubyte.gz
curl 'http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz' > t10k-labels-idx1-ubyte.gz
gzip -d train-images-idx3-ubyte.gz
gzip -d train-labels-idx1-ubyte.gz
gzip -d t10k-images-idx3-ubyte.gz
gzip -d t10k-labels-idx1-ubyte.gz
echo Dataset directory:
pwd
```

Now set DATASET_PATH in 'game_scripts/datasets/mnist.lua' to the directory
printed above.
