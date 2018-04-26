# Cifar10 dataset download instructions

The CIFAR-10 dataset consists of 60000 32x32 colour images in 10 classes,
with 6000 images per class. There are 50000 training images and 10000 test images.
They were collected by Alex Krizhevsky, Vinod Nair, and Geoffrey Hinton.
Website: https://www.cs.toronto.edu/~kriz/cifar.html

Make `data/cifar10` (the directory containing this file) the current directory
and run the following shell commands:

```sh
curl https://www.cs.toronto.edu/\~kriz/cifar-10-binary.tar.gz -o cifar-10-binary.tar.gz
tar -xvzf cifar-10-binary.tar.gz
mv cifar-10-batches-bin/*.* ./
rm -rf cifar-10-batches-bin cifar-10-binary.tar.gz

echo Dataset directory:
pwd
```

Now set DATASET_PATH in 'game_scripts/datasets/cfar10.lua' to the directory
printed above.
