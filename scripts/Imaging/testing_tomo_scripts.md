# Contents

<!-- TOC -->

- [Contents](#contents)
- [Utility](#utility)
    - [Merge pre-processing images into a stack. This will not apply any filters as pre-processing, it will just pack all of the images into a stack](#merge-pre-processing-images-into-a-stack-this-will-not-apply-any-filters-as-pre-processing-it-will-just-pack-all-of-the-images-into-a-stack)
- [Find Center Runs](#find-center-runs)
    - [Single image and no crop](#single-image-and-no-crop)
    - [Full `RB000888 test stack larmor summed 201510`, spheres crop](#full-rb000888-test-stack-larmor-summed-201510-spheres-crop)
- [Run Reconstruction](#run-reconstruction)
    - [Single Image <br>](#single-image-br)
    - [Full `RB000888 test stack larmor summed 201510`, bolts crop, good air region](#full-rb000888-test-stack-larmor-summed-201510-bolts-crop-good-air-region)
    - [Full `RB000888 test stack larmor summed 201510`, spheres crop, good air region](#full-rb000888-test-stack-larmor-summed-201510-spheres-crop-good-air-region)
    - [Full `RB000888_test_stack_larmor_summed_201510` dataset, bolts crop, **BAD** air region](#full-rb000888_test_stack_larmor_summed_201510-dataset-bolts-crop-bad-air-region)
- [ImageJ `GetSelectionCoordinates` Macro](#imagej-getselectioncoordinates-macro)
- [Python local tests](#python-local-tests)
    - [Pyfits load image stack](#pyfits-load-image-stack)
    - [Test loading single images and image stack](#test-loading-single-images-and-image-stack)
- [Plot Circular Mask](#plot-circular-mask)
- [Normalise by background comparison](#normalise-by-background-comparison)
- [Astra Reconstructions](#astra-reconstructions)
- [Wrong tool/algorithm tests](#wrong-toolalgorithm-tests)
- [SciPy ndimage zoom](#scipy-ndimage-zoom)
- [SciPy misc imresize](#scipy-misc-imresize)
- [SciPy timeit misc.imresize vs ndimage.zoom](#scipy-timeit-miscimresize-vs-ndimagezoom)
    - [Bigger data test](#bigger-data-test)
- [`Helper` class initialisation test](#helper-class-initialisation-test)
- [Tomo Test runs with as most args as possible](#tomo-test-runs-with-as-most-args-as-possible)
    - [--only-preproc](#--only-preproc)
    - [--reuse-preproc](#--reuse-preproc)
    - [--find-cor](#--find-cor)
    - [--crop-before-normalise](#--crop-before-normalise)
- [Testing the Big Data](#testing-the-big-data)
- [SCARF Chamber Tomo Find COR](#scarf-chamber-tomo-find-cor)
- [Type Conversion](#type-conversion)
    - [FITS > FITS](#fits--fits)
        - [Single > Stack, c1s](#single--stack-c1s)
        - [Stack > Single, c1](#stack--single-c1)
    - [FITS > NXS, this requires also dark and flat images](#fits--nxs-this-requires-also-dark-and-flat-images)
        - [AssertionError: Fail because no flat/dark, c2f](#assertionerror-fail-because-no-flatdark-c2f)
        - [Single > Stack, c2s](#single--stack-c2s)
        - [Stack > Stack, c2s2](#stack--stack-c2s2)
    - [NXS > FITS](#nxs--fits)
        - [Stack > Single, c3](#stack--single-c3)
        - [Stack > Stack, c3s](#stack--stack-c3s)
    - [TIFF > FITS](#tiff--fits)
        - [Single > Single, c4](#single--single-c4)
        - [Single > Stack, c4s](#single--stack-c4s)
        - [Stack > Single, c42](#stack--single-c42)
        - [Stack > Stack, c4s2](#stack--stack-c4s2)
    - [FITS > TIFF](#fits--tiff)
        - [Single > Single, c5](#single--single-c5)
        - [Single > Stack, c5s](#single--stack-c5s)
        - [Stack > Single, c52](#stack--single-c52)
        - [Stack > Stack, c5s2](#stack--stack-c5s2)
    - [TIFF > NXS](#tiff--nxs)
        - [Single > Stack, c6s](#single--stack-c6s)
        - [Stack > Stack](#stack--stack)
    - [NXS > TIFF](#nxs--tiff)
- [Pre-processing Data Flow with Dark/Flat and Median size 3](#pre-processing-data-flow-with-darkflat-and-median-size-3)
    - [FITS](#fits)
        - [Single > Single, p1](#single--single-p1)
        - [Single > Stack, p2](#single--stack-p2)
        - [Stack > Single, p3](#stack--single-p3)
        - [Stack > Stack, p4](#stack--stack-p4)
    - [NXS > FITS](#nxs--fits-1)
        - [Stack > Single, p5](#stack--single-p5)
        - [Stack > Stack, p5s](#stack--stack-p5s)
    - [FITS > NXS](#fits--nxs)
        - [Single > Stack, p6s](#single--stack-p6s)
    - [TIFF > FITS](#tiff--fits-1)
        - [Single > Single, p7](#single--single-p7)
        - [Single > Stack, p7s](#single--stack-p7s)
    - [TIFF > NXS](#tiff--nxs-1)
        - [Single > Stack](#single--stack)
        - [Stack > Stack](#stack--stack-1)

<!-- /TOC -->
 

# Utility
## Merge pre-processing images into a stack. This will not apply any filters as pre-processing, it will just pack all of the images into a stack

```python
python main.py -i=~/Documents/img/000888/data_full -o=~/Documents/img/test_stack/ --data-as-stack --convert
```
For Copy/Paste to terminal:
>python main.py -i=~/Documents/img/000888/data_full -o=~/Documents/img/000888/processed/temp/1 --data-as-stack


<br/>

# Find Center Runs

## Single image and no crop

```python
python main.py  
--num-iter=5  
--input-path=~/Documents/img/000888/data_single  
--input-path-flat=~/Documents/img/000888/flat  
--input-path-dark=~/Documents/img/000888/dark  
--region-of-interest='[0.000000, 0.000000, 511.000000, 511.000000]'  
--rotation=1  
--max-angle=360.000000  
--find-cor  
--output=~/Documents/img/000888/processed/temp/1  
--tool=tomopy 
```

For Copy/Paste to terminal:
>python main.py --num-iter=5 --input-path=~/Documents/img/000888/data_single --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[0.000000, 0.000000, 511.000000, 511.000000]' --rotation=1 --max-angle=360.000000 --find-cor --output=~/Documents/img/000888/processed/temp/1 --tool=tomopy 

EXPECTED RESULTS:
> COR: 265.0

---

## Full `RB000888 test stack larmor summed 201510`, spheres crop

```python
python main.py  
-n=5  
-i=~/Documents/img/000888/data_full  
-iflat=~/Documents/img/000888/flat  
-idark=~/Documents/img/000888/dark  
--region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]'  
--rotation=1  
--max-angle=360.000000  
--find-cor -t=tomopy  
--output=~/Documents/img/000888/processed/temp/1
```

>python main.py -n=5 -i=~/Documents/img/000888/data_full -iflat=~/Documents/img/000888/flat -idark=~/Documents/img/000888/dark --region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]' --rotation=1 --max-angle=360.000000 --find-cor -t=tomopy --output=~/Documents/img/000888/processed/temp/1

EXPECTED RESULTS:

> COR: 136.0 <br>
> Memory usage: 175484 KB, 171.37109375 MB <br>

<br />

# Run Reconstruction

## Single Image <br>

```python
python main.py
--tool=tomopy
--algorithm=gridrec
--num-iter=5
--input-path=~/Documents/img/000888/data_single
--input-path-flat=~/Documents/img/000888/flat
--input-path-dark=~/Documents/img/000888/dark
--region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]'
--output=~/Documents/img/000888/processed/temp/1
--median-size=3
--cor=255.000000
--rotation=1
--max-angle=360.000000
--data-as-stack
```
For Copy/Paste to terminal:
>python main.py --tool=tomopy --algorithm=gridrec --num-iter=5 --input-path=~/Documents/img/000888/data_single --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]' --output=~/Documents/img/000888/processed/temp/1 --median-size=3 --cor=255.000000 --rotation=1 --max-angle=360.000000 --data-as-stack

---

## Full `RB000888 test stack larmor summed 201510`, bolts crop, good air region
- ### Full `RB000888_test_stack_larmor_summed_201510` dataset <br>
- ### **OUT OF BOUNDS** Air Region if `--crop-before-normalise` is SPECIFIED <br>
- ### Better results/Air Region if run wihout `--crop-before-normalise`

```python
python main.py 
--tool=tomopy
--algorithm=gridrec
--num-iter=5
--input-path=~/Documents/img/000888/data_full
--input-path-flat=~/Documents/img/000888/flat
--input-path-dark=~/Documents/img/000888/dark
-R 41 0 233 228
--output=~/Documents/img/000888/processed/temp/1
--pre-median-size=3
--cor=104.000000
--rotation=1
--max-angle=360.000000
-A 360 111 388 144
-R 41 0 233 228
-data-as-stack
```
For Copy/Paste to terminal:
>python main.py --tool=tomopy --algorithm=gridrec --num-iter=5 --input-path=~/Documents/img/000888/data_full --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[41.0, 0.0, 230.0, 228.0]' --output=~/Documents/img/000888/processed/temp/1 --pre-median-size=3 --cor=104.000000 --rotation=1 --max-angle=360.000000 --air-region='[360.0, 111.0, 388.0, 144.0]' --data-as-stack

---

## Full `RB000888 test stack larmor summed 201510`, spheres crop, good air region
- ### Full `RB000888_test_stack_larmor_summed_201510` dataset <br>
- ### **OUT OF BOUNDS** Air Region if `--crop-before-normalise` is SPECIFIED <br>
- ### Better results/Air Region if run wihout `--crop-before-normalise`

```python
python main.py 
--tool=tomopy
--algorithm=gridrec
--num-iter=5
--input-path=~/Documents/img/000888/data_full
--input-path-flat=~/Documents/img/000888/flat
--input-path-dark=~/Documents/img/000888/dark
--region-of-interest='[35.0, 232.0, 224.0, 509.0]'
--output=~/Documents/img/000888/processed/temp/1
--median-size=3
--cor=136.000000
--rotation=1
--max-angle=360.000000
--air-region='[360.0, 111.0, 388.0, 144.0]'
--data-as-stack
```
For Copy/Paste to terminal:
>python main.py --tool=tomopy --algorithm=gridrec --num-iter=5 --input-path=~/Documents/img/000888/data_full --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[35.0, 232.0, 224.0, 509.0]' --output=~/Documents/img/000888/processed/temp/1 --median-size=3 --cor=136.000000 --rotation=1 --max-angle=360.000000 --air-region='[360.0, 111.0, 388.0, 144.0]' --data-as-stack
---

## Full `RB000888_test_stack_larmor_summed_201510` dataset, bolts crop, **BAD** air region
- ### ROI Crop **[36, 0, 219, 229]** <br>
- ### **WORKING** Air Region **[189.000000, 100.000000, 209.000000, 135.000000]** for crop if `--crop-before-normalise` is SPECIFIED <div id='id-section1'/>

```python
python main.py
--tool=tomopy
--algorithm=gridrec
--num-iter=5
--input-path=~/Documents/img/000888/data_full
--input-path-flat=~/Documents/img/000888/flat
--input-path-dark=~/Documents/img/000888/dark
--region-of-interest='[35.0, 232.0, 224.0, 509.0]'
--output=~/Documents/img/000888/processed/temp/1
--median-size=3
--cor=104.0
--rotation=1
--max-angle=360.000000
--air-region='[189.000000, 100.000000, 209.000000, 135.000000]'
--crop-before-normalise --data-as-stack
```
For Copy/Paste to terminal:
>python main.py --tool=tomopy --algorithm=gridrec --num-iter=5 --input-path=~/Documents/img/000888/data_full --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[35.0, 232.0, 224.0, 509.0]' --output=~/Documents/img/000888/processed/temp/1 --median-size=3 --cor=104.0 --rotation=1 --max-angle=360.000000 --air-region='[189.000000, 100.000000, 209.000000, 135.000000]' --crop-before-normalise --data-as-stack

<br/>

# ImageJ `GetSelectionCoordinates` Macro
- ### Gets selection coordinates and prints them in appropriate format to be copy pasted into Terminal

```
macro "List XY Coordinates" {
  requires("1.30k");
  getSelectionCoordinates(x, y);
  print("\'["+x[0]+".0, "+y[1]+".0, "+x[1]+".0, "+y[2]+".0]\'")
}
```
<br/>

# Python local tests

## Pyfits load image stack
```python
import pyfits
pyfits.open('/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/processed/gridrec/pre_processed/out_preproc_proj_images_stack.fits')
```

## Test loading single images and image stack
```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack

# load single images
sample = loader.load_stack('~/Documents/img/000888/data_full', argument_data_dtype=np.float32)[0]
rsample = rotate_stack._rotate_stack(sample, 3)
plt.imshow(rsample[0,232:509,35:224], cmap='Greys_r')  # spheres
plt.show()

plt.imshow(rsample[0,0:228,41:233], cmap='Greys_r')  # bolts
plt.show()

csample = rsample[:, 0:228,41:233]
sinograms = np.swapaxes(csample, 0, 1)

plt.imshow(rsample[:,0,:], cmap='Greys_r') # sinogram
plt.show()


import tomopy 
plt.imshow(tomopy.circ_mask(csample, axis=0, ratio=0.98)[0, :, :]); plt.show()
# load a stack of images
sample = loader.load_stack('/media/matt/Windows/Documents/mantid_workspaces/imaging/RB000888_test_stack_larmor_summed_201510/processed/gridrec/pre_processed', argument_data_dtype=np.float32)[0]
```

# Plot Circular Mask
```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack

# load single images
sample = loader.load_stack('~/Documents/img/000888/data_full', argument_data_dtype=np.float32)[0]
rsample = rotate_stack._rotate_stack(sample, 3)
csample = rsample[:, 0:228,41:233]
import tomopy 
plt.imshow(tomopy.circ_mask(csample, axis=0, ratio=0.98)[0], cmap='Greys_r'); plt.show()
from recon.filters import circular_mask
plt.imshow(circular_mask.execute_custom(csample, 0.98)[0], cmap='Greys_r'); plt.show()
```

# Normalise by background comparison
```python
from recon.data import loader
from recon.filters import normalise_by_flat_dark
from recon.filters import rotate_stack as rs
import numpy as np
import matplotlib.pyplot as plt
import tomopy
from recon.configs.recon_config import ReconstructionConfig

config = ReconstructionConfig.emtpy_init()

sample, flat, dark = loader.load_stack(sample_path='~/Documents/img/000888/data_full', flat_file_path='~/Documents/img/000888/flat', dark_file_path='~/Documents/img/000888/dark', argument_data_dtype=np.float32)
tsample, tflat, tdark = loader.load_stack(sample_path='~/Documents/img/000888/data_full', flat_file_path='~/Documents/img/000888/flat', dark_file_path='~/Documents/img/000888/dark', argument_data_dtype=np.float32)
r = 3

sample = rs._rotate_stack(sample, r)
tsample = rs._rotate_stack(tsample, r)
flat = rs._rotate_image(flat, r)
tflat = rs._rotate_image(tflat, r)
dark = rs._rotate_image(dark, r)
tdark = rs._rotate_image(tdark, r)

sample = normalise_by_flat_dark.execute(sample, config, flat, dark)
tsample = tomopy.normalize(tsample, flat, dark)

plt.imshow(np.concatenate((sample[0], tsample[0]), axis=1), cmap='Greys_r'); plt.show()

```

# Astra Reconstructions
`astra_create_proj_geom`:
- det_spacing: distance between the centers of two adjacent detector pixels (0.55?)
- det_count: number of detector pixels in a single projection (512?)
- angles: projection angles in radians

```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack

# load single images
sample = loader.load_stack('~/Documents/img/000888/data_full', argument_data_dtype=np.float32)[0]
rsample = rotate_stack._rotate_stack(sample, 3)
csample = rsample[:, 0:228,41:233]
num_proj = csample.shape[0]
inc = float(360.0) / num_proj
proj_angles = np.arange(0, num_proj * inc, inc)
proj_angles = np.radians(proj_angles)

vol_geom = astra.create_vol_geom(512)
proj_geom = astra.create_proj_geom('parallel', 0.55, 512, proj_angles)

```

# Wrong tool/algorithm tests

```python
python main.py 
-i=~/Documents/img/000888/data_single 
--input-path-flat=~/Documents/img/000888/flat 
--input-path-dark=~/Documents/img/000888/dark 
--region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]' 
-o=~/Documents/img/000888/processed/temp/1 
--median-size=3 
--cor=255.000000 
--rotation=1 
--max-angle=360.000000 
--data-as-stack 
-t tomopy # just change tool
-a afewaf # or or algorithm
```

> python main.py -i=~/Documents/img/000888/data_single --input-path-flat=~/Documents/img/000888/flat --input-path-dark=~/Documents/img/000888/dark --region-of-interest='[36.000000, 227.000000, 219.000000, 510.000000]' -o=~/Documents/img/000888/processed/temp/1 --median-size=3 --cor=255.000000 --rotation=1 --max-angle=360.000000 --data-as-stack -t tomopy -a afewaf


# SciPy ndimage zoom
```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack
import scipy.ndimage as sn

# load single images
sample = loader.load_stack('~/Documents/img/000888/data_full', argument_data_dtype=np.float32)[0]
rsample = rotate_stack._rotate_stack(sample, 3)
print(rsample.shape)
rebin = 0.6565
num_images = rsample.shape[0]
expected_dims = round(rsample.shape[1]*rebin)  # this will give the shape calculated by scipy
boop = np.zeros((num_images, expected_dims, expected_dims), dtype=np.float32)
print(boop.shape) 
for idx in xrange(rsample.shape[0]):
    boop[idx] = sn.zoom(rsample[idx], rebin)
    rsample[idx] = 0

plt.imshow(boop[0], cmap='Greys_r'); plt.show()
```

# SciPy misc imresize
```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack
import scipy.misc as sm
# load single images
sample = loader.load_stack('~/Documents/img/000888/data_full', argument_data_dtype=np.float32)[0]
rsample = rotate_stack._rotate_stack(sample, 3)
print(rsample.shape)
rebin = 1.5
num_images = rsample.shape[0]
expected_dims = round(rsample.shape[1]*rebin)  # this will give the shape calculated by scipy
boop = np.zeros((num_images, expected_dims, expected_dims), dtype=np.float32)
print(boop.shape) 
for idx in xrange(rsample.shape[0]):
    boop[idx] = sm.imresize(rsample[idx], rebin, interp='bicubic')
    rsample[idx] = 0

plt.imshow(boop[0], cmap='Greys_r'); plt.show()

```

# SciPy timeit misc.imresize vs ndimage.zoom

```python
from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack
import scipy.misc as sm
import scipy.ndimage as sn
from recon.helper import Helper

sample = loader.load_stack("~/Documents/img/000888/data_full")[0]

def imresize(sample):
    boop = np.zeros((143, 336, 336), dtype=np.float32)
    
    for idx in xrange(143):
        boop[idx] = sm.imresize(sample[idx], 0.6565, interp='nearest')
        sample[idx] = 0


def zoom(sample):
    
    boop = np.zeros((143, 336, 336), dtype=np.float32)
    for idx in xrange(143):
        boop[idx] = sn.zoom(sample[idx], 0.6565)


import timeit
timeit.timeit(stmt='imresize(sample)', setup='from __main__ import sample, loader, imresize; import numpy as np; gc.enable()', number=100)
timeit.timeit(stmt='zoom(sample)', setup='from __main__ import sample, loader, zoom; import numpy as np; gc.enable()', number=100)
```

## Bigger data test
```python

from recon.data import loader
import numpy as np
import matplotlib.pyplot as plt
from recon.filters import rotate_stack
import scipy.misc as sm
import scipy.ndimage as sn
from recon.helper import Helper

sample = loader.load_stack("~/Documents/img/000888/data_full")[0]
sample = np.concatenate(sample, sample)
def imresize(sample):
    boop = np.zeros((143, 336, 336), dtype=np.float32)
    
    for idx in xrange(143):
        boop[idx] = sm.imresize(sample[idx], 0.6565, interp='nearest')
        sample[idx] = 0


def zoom(sample):
    
    boop = np.zeros((143, 336, 336), dtype=np.float32)
    for idx in xrange(143):
        boop[idx] = sn.zoom(sample[idx], 0.6565)


import timeit
timeit.timeit(stmt='imresize(sample)', setup='from __main__ import sample, loader, imresize; import numpy as np; gc.enable()', number=100)
timeit.timeit(stmt='zoom(sample)', setup='from __main__ import sample, loader, zoom; import numpy as np; gc.enable()', number=100)
```

# `Helper` class initialisation test
```python
python -c "from recon.helper import Helper; g=[]; h=Helper(); h=Helper(g)"
```

# Tomo Test runs with as most args as possible

## --only-preproc

> python main.py -i ~/Documents/img/000888/data_full -l ~/Documents/img/000888/flat -k ~/Documents/img/000888/dark -o ~Documents/img/000888/processed/temp/1 -s -w -c 104.0 -t tomopy -a fbp -n 5 -g '[35.0, 232.0, 224.0, 509.0]' -e '[189.000000, 100.000000, 209.000000, 135.000000]' -r 1 -v 3 -d --pre-median-size=3 --pre-median-mode='wrap' --data-dtype='float32' --max-angle=360.0 --rebin 0.5 --rebin-mode 'bicubic' --circular-mask 0.96 --clip-min 0 --clip-max 1.5 --cut-off-pre 0.01 --cut-off-post 0.01 --out-gaussian-size 3 --out-gaussian-mode mirror --out-median-size=3 --out-median-mode='wrap' --data-as-stack --save-horiz --only-preproc

## --reuse-preproc

> python main.py -i ~/Documents/img/000888/data_full -l ~/Documents/img/000888/flat -k ~/Documents/img/000888/dark -o ~Documents/img/000888/processed/temp/1 -s -w -c 104.0 -t tomopy -a fbp -n 5 -g '[35.0, 232.0, 224.0, 509.0]' -e '[189.000000, 100.000000, 209.000000, 135.000000]' -r 1 -v 3 -d --pre-median-size=3 --pre-median-mode='wrap' --data-dtype='float32' --max-angle=360.0 --rebin 0.5 --rebin-mode 'bicubic' --circular-mask 0.96 --clip-min 0 --clip-max 1.5 --cut-off-pre 0.01 --cut-off-post 0.01 --out-gaussian-size 3 --out-gaussian-mode mirror --out-median-size=3 --out-median-mode='wrap' --data-as-stack --save-horiz --only-preproc --reuse-preproc

## --find-cor

> python main.py -i ~/Documents/img/000888/data_full -l ~/Documents/img/000888/flat -k ~/Documents/img/000888/dark -o ~Documents/img/000888/processed/temp/1 -s -w -c 104.0 -t tomopy -a fbp -n 5 -g '[35.0, 232.0, 224.0, 509.0]' -e '[189.000000, 100.000000, 209.000000, 135.000000]' -r 1 -v 3 -d --pre-median-size=3 --pre-median-mode='wrap' --data-dtype='float32' --max-angle=360.0 --rebin 0.5 --rebin-mode 'bicubic' --circular-mask 0.96 --clip-min 0 --clip-max 1.5 --cut-off-pre 0.01 --cut-off-post 0.01 --out-gaussian-size 3 --out-gaussian-mode mirror --out-median-size=3 --out-median-mode='wrap' --data-as-stack --save-horiz --only-preproc -f 

## --crop-before-normalise
> python main.py -i ~/Documents/img/000888/data_full -l ~/Documents/img/000888/flat -k ~/Documents/img/000888/dark -o ~Documents/img/000888/processed/temp/1 -s -w -c 104.0 -t tomopy -a fbp -n 5 -g '[35.0, 232.0, 224.0, 509.0]' -e '[189.000000, 100.000000, 209.000000, 135.000000]' -r 1 -v 3 -d --pre-median-size=3 --pre-median-mode='wrap' --data-dtype='float32' --max-angle=360.0 --rebin 0.5 --rebin-mode 'bicubic' --circular-mask 0.96 --clip-min 0 --clip-max 1.5 --cut-off-pre 0.01 --cut-off-post 0.01 --out-gaussian-size 3 --out-gaussian-mode mirror --out-median-size=3 --out-median-mode='wrap' --data-as-stack --save-horiz --crop-before-normalise 

# Testing the Big Data
To drop caches for real performance tests: `alias drop_caches='echo 3 | sudo tee /proc/sys/vm/drop_caches'`

Windows Path: /media/matt/Windows/Documents/mantid_workspaces/imaging/chamber/

`python main.py -i /media/matt/Windows/Documents/mantid_workspaces/imaging/chamber/ -o /media/matt/Windows/Documents/mantid_workspaces/imaging/chamber/processed/temp/1 --data-as-stack --only-preproc`
Stats:
Images: 500
Disk Read: ~130MB/s
Time: ~35s
Memory: 8016 MB
Data Type: float32

Linux Path: ~/Documents/img/chamber/
Images: 500
Disk: ~130MB/s
Time: ~35s
Memory: 8016  MB
Data Type: float32

# SCARF Chamber Tomo Find COR

```python 
python main.py 
bsub -I python /work/imat/imat_recon/scripts/main.py 

-i /work/imat/chamber_tomo/temp/full_stack/ -k /work/imat/chamber_tomo/Dark/ -l /work/imat/chamber_tomo/Flat/ -o /work/imat/chamber_tomo/temp/full_preproc --out-format fits --only-preproc --data-as-stack -w --in-format fits -R 384 0 1550 1932 -A 384 686 476 804 --pre-median-size 3
```

>python main.py -i /media/matt/Windows/Documents/mantid_workspaces/imaging/chamber/temp/1000/pre_processed -o /media/matt/Windows/Documents/mantid_workspaces/imaging/chamber/processed/temp/1000_processed -g '[384.0, 0.0, 1550.0, 1932.0]' -f

# Type Conversion

## FITS > FITS
### Single > Stack, c1s
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/c1s -s --convert --data-as-stack`
### Stack > Single, c1
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/c1 -s --convert`

## FITS > NXS, this requires also dark and flat images
### AssertionError: Fail because no flat/dark, c2f
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/c2f -s --convert --data-as-stack --out-format nxs`
### Single > Stack, c2s
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/c2s -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --convert --out-format nxs --data-as-stack`
### Stack > Stack, c2s2
`python main.py -i ~/temp/c1s/pre_processed/ -o ~/temp/c2s2 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --convert --out-format nxs --data-as-stack`

## NXS > FITS
### Stack > Single, c3
`python main.py -i ~/temp/c2s/pre_processed/ -o ~/temp/c3 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --convert --in-format nxs`
### Stack > Stack, c3s
`python main.py -i ~/temp/c2s/pre_processed/ -o ~/temp/c3s -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --convert --in-format nxs --data-as-stack`

## TIFF > FITS
### Single > Single, c4
`python main.py -i ~/win_img/777cannon/data/ -o ~/temp/c4 -s --convert --in-format tiff`
### Single > Stack, c4s
`python main.py -i ~/win_img/777cannon/data/ -o ~/temp/c4 -s --convert --data-as-stack --in-format tiff`
### Stack > Single, c42
`TODO currently we cannot save out TIFF files`
### Stack > Stack, c4s2
`TODO currently we cannot save out TIFF files`

## FITS > TIFF
### Single > Single, c5
`TODO currently fails with ValueError: Images of type float must be between -1 and 1.`
### Single > Stack, c5s
`TODO currently fails with ValueError: Images of type float must be between -1 and 1.`
### Stack > Single, c52
`TODO currently fails with ValueError: Images of type float must be between -1 and 1.`
### Stack > Stack, c5s2
`TODO currently fails with ValueError: Images of type float must be between -1 and 1.`

## TIFF > NXS
### Single > Stack, c6s

### Stack > Stack
`TODO currently we cannot save out TIFF files`

## NXS > TIFF
`TODO currently we cannot save out TIFF files`

# Pre-processing Data Flow with Dark/Flat and Median size 3
## FITS
### Single > Single, p1
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/p1 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --pre-median-size 3 --only-preproc`
### Single > Stack, p2
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/p2 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --pre-median-size 3 --only-preproc --data-as-stack`
### Stack > Single, p3
`python main.py -i ~/temp/c1/pre_processed/ -o ~/temp/p3 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --pre-median-size 3 --only-preproc`
### Stack > Stack, p4
`python main.py -i ~/temp/c1/pre_processed/ -o ~/temp/p4 -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --pre-median-size 3 --only-preproc --data-as-stack`

## NXS > FITS
### Stack > Single, p5
`python main.py -i ~/temp/c6s_new/pre_processed/ --only-preproc -o ~/temp/p4/ --in-format nxs --pre-median-size 3 -s`

### Stack > Stack, p5s
`python main.py -i ~/temp/c6s_new/pre_processed/ --only-preproc -o ~/temp/p4/ --in-format nxs --pre-median-size 3 -s --data-as-stack`

## FITS > NXS
### Single > Stack, p6s
`python main.py -i ~/win_img/larmor/data/ -o ~/temp/c2s -D ~/win_img/larmor/dark/ -F ~/win_img/larmor/flat/ -s --only-preproc --out-format nxs --data-as-stack --pre-median-size 3`

## TIFF > FITS
### Single > Single, p7
`python main.py -i ~/win_img/777cannon/data/ -o ~/temp/p7 -D ~/win_img/777cannon/dark_cannon/ -F ~/win_img/777cannon/flat_cannon/ -s --only-preproc --in-format tif --pre-median-size 3`

### Single > Stack, p7s
`python main.py -i ~/win_img/777cannon/data/ -o ~/temp/p7s -D ~/win_img/777cannon/dark_cannon/ -F ~/win_img/777cannon/flat_cannon/ -s --only-preproc --in-format tif --pre-median-size 3 --data-as-stack`

## TIFF > NXS
### Single > Stack
### Stack > Stack
