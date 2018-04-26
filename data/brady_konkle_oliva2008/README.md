# Stimuli dataset download instructions

Object stimuli from: Brady, T. F., Konkle, T., Alvarez, G. A. and Oliva, A. (2008).
Visual long-term memory has a massive storage capacity for object details.
Proceedings of the National Academy of Sciences, USA, 105 (38), 14325-14329.

Make `data/brady_konkle_oliva2008` (the directory containing this file) the
current directory and run the following shell commands:

```sh
curl https://bradylab.ucsd.edu/stimuli/ObjectsAll.zip -o ObjectsAll.zip
unzip ObjectsAll.zip
pushd OBJECTSALL

python << EOM
import os
from PIL import Image
files = [f for f in os.listdir('.') if f.endswith('jpg') or f.endswith('JPG')]
for i, file in enumerate(sorted(files)):
  im = Image.open(file)
  im.save('../%04d.png' % (i+1))
EOM
popd
rm -rf __MACOSX OBJECTSALL ObjectsAll.zip

echo Dataset directory:
pwd
```

Now set DATASET_PATH in 'game_scripts/datasets/brady_konkle_oliva2008.lua' to
the directory printed above.
