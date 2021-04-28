import os, sys
import cv2
import numpy as np
import torch
from tqdm import tqdm
from yacs.config import CfgNode as CN

from decalib.datasets import datasets 
from decalib.utils import util
from decalib.utils.renderer import SRenderY
from decalib.models.encoders import ResnetEncoder
from decalib.models.FLAME import FLAME
from decalib.models.decoders import Generator

@torch.no_grad()
def encode(flame_encoder, images):
    parameters = flame_encoder(images)
    codedict = decompose_code(parameters, {'shape': 100, 'tex': 50, 'exp': 50, 'pose': 6, 'cam': 3, 'light': 27})
    codedict['images'] = images
    return codedict

def decompose_code(code, num_dict):
    code_dict = {}
    start = 0
    for key in num_dict:
        end = start+int(num_dict[key])
        code_dict[key] = code[:, start:end]
        start = end
    return code_dict

@torch.no_grad()
def decode(flame_decoder, codedict, renderer):
    images = codedict['images']
    verts, landmarks2d, landmarks3d = flame_decoder(shape_params=codedict['shape'])
    landmarks2d = util.batch_orth_proj(landmarks2d, codedict['cam'])[:,:,:2]
    landmarks2d[:,:,1:] = -landmarks2d[:,:,1:]
    landmarks2d = landmarks2d * 112 + 112
    landmarks3d = util.batch_orth_proj(landmarks3d, codedict['cam'])
    landmarks3d[:,:,1:] = -landmarks3d[:,:,1:]
    landmarks3d = landmarks3d * 112 + 112
    trans_verts = util.batch_orth_proj(verts, codedict['cam'])
    trans_verts[:,:,1:] = -trans_verts[:,:,1:]
    shape_images = renderer.render_shape(verts, trans_verts)
    opdict = {
        'vertices': verts,
        'transformed_vertices': trans_verts,
        'landmarks2d': landmarks2d,
        'landmarks3d': landmarks3d
    }
    visdict = {
        'inputs': images, 
        'landmarks2d': util.tensor_vis_landmarks(images, landmarks2d, isScale=False),
        'landmarks3d': util.tensor_vis_landmarks(images, landmarks3d, isScale=False),
        'shape_images': shape_images
    }
    return opdict, visdict

savefolder = 'results'
os.makedirs(savefolder, exist_ok=True)

flame_config = CN()
flame_config.flame_model_path = 'data/generic_model.pkl'
flame_config.flame_lmk_embedding_path = 'data/landmark_embedding.npy'
flame_config.n_shape = 100
flame_config.n_exp = 50

#load flat images
testdata = datasets.TestData('input', face_detector='fan')

#instantiate objects
flame_encoder = ResnetEncoder(outsize=1).to('cuda') #TODO: check outsize number
flame_decoder = FLAME(flame_config).to('cuda')
renderer = SRenderY(224, obj_filename='data/head_template.obj').to('cuda')

#load model
checkpoint = torch.load('data/deca_model.tar')
util.copy_state_dict(flame_encoder.state_dict(), checkpoint['E_flame'])
flame_encoder.eval()

name = testdata[0]['imagename']
images = testdata[0]['image'].to('cuda')[None,...]

codedict = encode(flame_encoder, images)
opdict, visdict = decode(flame_decoder, codedict, renderer)

os.makedirs(os.path.join(savefolder, name), exist_ok=True)
np.savetxt(os.path.join(savefolder, name, name + '_kpt2d.txt'), opdict['landmarks2d'].cpu().numpy())
np.savetxt(os.path.join(savefolder, name, name + '_kpt3d.txt'), opdict['landmarks3d'].cpu().numpy())

vertices = opdict['vertices'][0].cpu().numpy()
faces = renderer.faces[0].cpu().numpy()
util.write_obj(os.path.join(savefolder, name, name + '.obj'), vertices, faces)

for vis_name in ['inputs', 'rendered_images', 'shape_images']:
    if vis_name not in visdict.keys():
        continue
    image = util.tensor2image(visdict[vis_name][0])
    cv2.imwrite(os.path.join(savefolder, name, name + '_' + vis_name +'.jpg'), util.tensor2image(visdict[vis_name][0]))
'''
for i in tqdm(range(len(testdata))):
    name = testdata[i]['imagename']
    images = testdata[i]['image'].to('cuda')[None,...]

    codedict = encode(flame_encoder, images)
    opdict, visdict = decode(flame_decoder, codedict, renderer)
    
    os.makedirs(os.path.join(savefolder, name), exist_ok=True)
    np.savetxt(os.path.join(savefolder, name, name + '_kpt2d.txt'), opdict['landmarks2d'].cpu().numpy())
    np.savetxt(os.path.join(savefolder, name, name + '_kpt3d.txt'), opdict['landmarks3d'].cpu().numpy())

    vertices = opdict['vertices'][i].cpu().numpy()
    faces = renderer.faces[0].cpu().numpy()
    util.write_obj(os.path.join(savefolder, name, name + '.obj'), vertices, faces)

    for vis_name in ['inputs', 'rendered_images', 'shape_images']:
        if vis_name not in visdict.keys():
            continue
        image = util.tensor2image(visdict[vis_name][0])
        cv2.imwrite(os.path.join(savefolder, name, name + '_' + vis_name +'.jpg'), util.tensor2image(visdict[vis_name][0]))
'''