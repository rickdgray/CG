# COSC 6372 Computer Graphics
Can be run just by using makefile. Uses round, max, min, pow in math.h and utilities so requires c++ 99 or later. Tested on ubuntu.

# 3D Mesh Generation and Landmark Transfer
This project will take flat images of human faces and generate 3D meshes, then transfer landmarks from other faces to distort the mesh.

### Setting up the environment
There's a lot. You _must_ have an NVIDIA GPU; it will not run without one. I also strongly recommend running on a modern linux distribution, though hypothetically it will run on other operating systems.
Running from a live linux session instead of a standard installation will almost certainly not work.
This was built and ran on Ubuntu 20 with a GTX 1080.

First, verify you have python3 and pip3 installed:
```bash
sudo apt install python3 python3-pip
```

Then, you will need to install the nvidia cuda toolkit. This generator was built on the Pytorch3D library which requires version 10.2. The latest version (11.3 at time of writing) is not backwards compatible.

On Ubuntu 20, running
```bash
sudo apt install nvidia-cuda-toolkit
```
installed the correct version, but this will depend on the repository. See the [CUDA archive](https://developer.nvidia.com/cuda-toolkit-archive) for installing older versions if necessary.

You can test that it is running properly by running the following command:
```bash
nvidia-smi
```

Then you may run
```bash
pip3 install -r requirements.txt
```
to install the requisite python packages. Note that the torch version is 1.7.1 and torchvision is 0.8.2 exclusively. Later versions are not backwards compatible with Pytorch3D, so these specific versions must be installed.

Finally, you can run 
```bash
pip3 install fvcore ioPath
export FORCE_CUDA=1
pip3 install "git+https://github.com/facebookresearch/pytorch3d.git"
```
to install [pytorch3d](https://github.com/facebookresearch/pytorch3d).

It is recommended to set the environment variable `FORCE_CUDA` to `1` so that compilation will fail if the cuda toolkit is not installed properly.

### Setting up model and input
This project builds on the [DECA](https://arxiv.org/abs/2012.04012) library and the [FLAME](https://flame.is.tue.mpg.de/home) generic models. We will use DECA's trained neural network and the generic mesh from FLAME to generate our _base faces_.

To do so, these two files must be prepared:
  1. The 'generic_model.pkl' file unzipped into the data folder from the [FLAME 2020](https://flame.is.tue.mpg.de/downloads) model pack
  2. The [DECA trained model](https://drive.google.com/file/d/1rp8kdyLPvErw2dTmqtjISRVvQLj6Yzje/view?usp=sharing) copied directly into the data folder as a .tar. It is not a standard tarball and tar will not decompress it.

Once the files are in place, a simple photo of a source face may be placed into the 'input/faces' folder and a source face for landmark transfer may be placed in the 'input/transfer' folder for processing. Photos cropped to around the face with a straight on view produce the best results.

Finally, to generate a mesh and renders, you can simply run
```bash
python3 gen_faces.py
```
from the root folder.
