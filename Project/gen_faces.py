import os
import cv2
import numpy as np
import torch
from tqdm import tqdm
from yacs.config import CfgNode as CN

from decalib.datasets import datasets
from decalib.models.encoders import ResnetEncoder
from decalib.models.FLAME import FLAME
from decalib.utils import util
from decalib.utils.renderer import SRenderY

@torch.no_grad()
def encode(flame_encoder, images, param_dict):
    parameters = flame_encoder(images)
    codedict = decompose_code(parameters, param_dict)
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
    verts, landmarks2d, landmarks3d = flame_decoder(shape_params=codedict['shape'], expression_params=codedict['exp'], pose_params=codedict['pose'])
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
        'landmarks2d': landmarks2d,
        'landmarks3d': landmarks3d
    }
    visdict = {
        'inputs': images,
        'shape_images': shape_images
    }
    return opdict, visdict

savefolder = 'results'
os.makedirs(savefolder, exist_ok=True)

flame_config = CN()
flame_config.flame_model_path = 'data/generic_model.pkl'
flame_config.flame_lmk_embedding_path = 'data/landmark_embedding.npy'
flame_config.param_list = ['shape', 'tex', 'exp', 'pose', 'cam', 'light']
flame_config.n_shape = 100
flame_config.n_tex = 50
flame_config.n_exp = 50
flame_config.n_pose = 6
flame_config.n_cam = 3
flame_config.n_light = 27

param_dict = {i:flame_config.get('n_' + i) for i in flame_config.param_list}

#load flat images
faceData = datasets.TestData('input/faces', face_detector='fan')
caricatureData = datasets.TestData('input/caricatures', face_detector='fan')

#instantiate objects
flame_encoder = ResnetEncoder(outsize=236).to('cuda')
flame_decoder = FLAME(flame_config).to('cuda')
renderer = SRenderY(224, obj_filename='data/head_template.obj').to('cuda')

#load neural network model
checkpoint = torch.load('data/deca_model.tar')
util.copy_state_dict(flame_encoder.state_dict(), checkpoint['E_flame'])
flame_encoder.eval()

for i in tqdm(range(len(faceData))):
    faceName = faceData[i]['imagename']
    faceImages = faceData[i]['image'].to('cuda')[None,...]

    caricatureImages = caricatureData[0]['image'].to('cuda')[None,...]

    faceCodedict = encode(flame_encoder, faceImages, param_dict)
    faceOpdict, faceVisdict = decode(flame_decoder, faceCodedict, renderer)

    caricatureCodedict = encode(flame_encoder, caricatureImages, param_dict)

    #transfer style
    faceCodedict['pose'][:,3:] = caricatureCodedict['pose'][:,3:]
    faceCodedict['exp'] = caricatureCodedict['exp']

    resultOpdict, resultVisdict = decode(flame_decoder, faceCodedict, renderer)
    
    os.makedirs(os.path.join(savefolder, faceName), exist_ok=True)
    np.savetxt(os.path.join(savefolder, faceName, faceName + '_kpt2d.txt'), faceOpdict['landmarks2d'][0].cpu().numpy())
    np.savetxt(os.path.join(savefolder, faceName, faceName + '_kpt3d.txt'), faceOpdict['landmarks3d'][0].cpu().numpy())

    vertices = faceOpdict['vertices'][0].cpu().numpy()
    faces = renderer.faces[0].cpu().numpy()
    util.write_obj(os.path.join(savefolder, faceName, faceName + '.obj'), vertices, faces)

    image = util.tensor2image(faceVisdict['inputs'][0])
    image = util.tensor2image(faceVisdict['shape_images'][0])
    image = util.tensor2image(resultVisdict['shape_images'][0])
    cv2.imwrite(os.path.join(savefolder, faceName, faceName + '_' + 'inputs' +'.jpg'), util.tensor2image(faceVisdict['inputs'][0]))
    cv2.imwrite(os.path.join(savefolder, faceName, faceName + '_' + 'shape_images' +'.jpg'), util.tensor2image(faceVisdict['shape_images'][0]))
    cv2.imwrite(os.path.join(savefolder, faceName, faceName + '_' + 'transferred_images' +'.jpg'), util.tensor2image(resultVisdict['shape_images'][0]))
