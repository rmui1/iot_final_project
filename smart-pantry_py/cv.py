import os
import torch

import torch
from torchvision.io import read_image

from torchvision.ops.boxes import masks_to_boxes
from torchvision import tv_tensors
from torchvision.transforms.v2 import functional as F

import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor
from torchvision.models.detection import FasterRCNN
from torchvision.models.detection.rpn import AnchorGenerator
from torchvision.models.detection.mask_rcnn import MaskRCNNPredictor

from torchvision.transforms import v2 as T

import matplotlib
import matplotlib.pyplot as plt
from torchvision.utils import draw_bounding_boxes, draw_segmentation_masks

import json

# based off of PyTorch object detection tutorial: https://pytorch.org/tutorials/intermediate/torchvision_tutorial.html

items = {1: 'pickled plum', 2: 'sweet corn', 3: 'spreadable honey'}

def get_model_instance_segmentation(num_classes):
    # load an instance segmentation model pre-trained on COCO
    model = torchvision.models.detection.maskrcnn_resnet50_fpn(weights="DEFAULT")

    # get number of input features for the classifier
    in_features = model.roi_heads.box_predictor.cls_score.in_features
    # replace the pre-trained head with a new one
    model.roi_heads.box_predictor = FastRCNNPredictor(in_features, num_classes)

    # now get the number of input features for the mask classifier
    in_features_mask = model.roi_heads.mask_predictor.conv5_mask.in_channels
    hidden_layer = 256
    # and replace the mask predictor with a new one
    model.roi_heads.mask_predictor = MaskRCNNPredictor(
        in_features_mask,
        hidden_layer,
        num_classes
    )

    return model

def get_transform(train):
    transforms = []
    # if train:
    #     transforms.append(T.RandomHorizontalFlip(0.5))
    transforms.append(T.ToDtype(torch.float, scale=True))
    transforms.append(T.ToPureTensor())
    return T.Compose(transforms)

def save_image_and_get_visible_food(image_path):
    # train on the GPU or on the CPU, if a GPU is not available
    device = torch.device('cuda') if torch.cuda.is_available() else torch.device('cpu')
    num_classes = 4

    # get the model using our helper function
    model = get_model_instance_segmentation(num_classes)

    # move model to the right device
    model.to(device)

    model.load_state_dict(torch.load('new_model.pt'))
    image = read_image(image_path) # ../iot/20231106082426.jpg
    eval_transform = get_transform(train=False)
    model.eval()

    with torch.no_grad():
        x = eval_transform(image)
        # convert RGBA -> RGB and move to device
        x = x[:3, ...].to(device)
        predictions = model([x, ])
        pred = predictions[0]

    image = (255.0 * (image - image.min()) / (image.max() - image.min())).to(torch.uint8)
    image = image[:3, ...]

    from torchvision.ops import nms
    filtered_boxes = nms(pred["boxes"], pred["scores"], iou_threshold = 0.1)

    pred_labels = [f"{items[int(label)]}" for label, score in zip(pred["labels"][filtered_boxes], pred["scores"][filtered_boxes])]
    pred_boxes = pred["boxes"][filtered_boxes].long()

    output_image = draw_bounding_boxes(image, pred_boxes, pred_labels, colors="red")

    masks = (pred["masks"] > 0.7).squeeze(1)
    output_image = draw_segmentation_masks(output_image, masks, alpha=0.5, colors="blue")

    # save resulting image
    matplotlib.use('agg')
    plt.figure(figsize=(12, 12))
    plt.imshow(output_image.permute(1, 2, 0))
    plt.savefig('static/result.jpg', bbox_inches='tight', pad_inches=0)

    return [items[int(l)] for l in pred["labels"][filtered_boxes]]

# print(save_image_and_get_visible_food('../iot/20231201094341.jpg'))