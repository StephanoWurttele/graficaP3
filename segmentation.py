import numpy as np
import os
save_path = "final"
f = open("coords.txt", "a")

def write_point(x,y,z):
    f.write(f"{str(float(x)/10.0)}\n")
    f.write(f"{str(float(y)/10.0)}\n")
    f.write(f"{str(float(z)/5.0)}\n")
    

def load_npy(path, segmentation = True):
    input_image = np.load(path)
    if(segmentation): return input_image
    input_image = np.moveaxis(input_image, 0, -1)
    input_image = np.moveaxis(input_image, 0, -1)
    return input_image

def has_white_up(sheet,y,x):
    return sheet[y-1][x] != 0

def has_white_down(sheet,y,x):
    if y == 511: return False
    return sheet[y+1][x] != 0

def get_initial(seg):
    for y in range(0,512):
        for x in range(0, 512):
            if(seg[y][x] == 100): return [y, x]

def write_ordered(seg, z):
    if(z==159): return
    y , x = get_initial(seg)
    while(True):
        write_point(x,y,z)
        seg[y][x] =  90
        if(seg[y][x+1] == 100): x += 1
        elif(seg[y-1][x+1] == 100): 
            x += 1
            y -= 1
        elif(seg[y+1][x+1] == 100):
            x += 1
            y += 1
        elif(seg[y-1][x] == 100): y -=1
        elif(seg[y+1][x] == 100): y +=1
        elif(seg[y][x-1] == 100): x -= 1
        elif(seg[y-1][x-1] == 100): 
            x -= 1
            y -= 1
        elif(seg[y+1][x-1] == 100):
            x -= 1
            y += 1
        else: break
    f.write("===\n")

def get_border_segmentation():
    path_segmented = "bare_segmentation.npy"
    print("Reading segmentation from", path_segmented)
    segmentation = load_npy(path_segmented)
    npy_segmented_dicom = segmentation
    only_border = npy_segmented_dicom.copy()
    print("---------------------------------------------------------------------------")
    print("Segmented npy    :",npy_segmented_dicom.shape)
    print("segmentation found in slices: ")
    exists_seg = []
    for i in range(len(npy_segmented_dicom-1)):
        sheet_segmentado = npy_segmented_dicom[i]
        if(np.sum(sheet_segmentado) == 0 ): continue
        for y in range(512):
            found = False
            if(np.sum(sheet_segmentado[y]) == 0): continue
            for x in range(512):
                if(not found):
                    if(sheet_segmentado[y][x] == 0.0): continue
                    found = True
                    # write_point(x,y,i)
                    only_border[i][y][x] = 100
                    if(sheet_segmentado[y][x+1] == 0.0): found = False
                    if i not in exists_seg: 
                        print(str(i) + ", ", end="")
                        exists_seg.append(i)
                else:
                    if(sheet_segmentado[y][x] != 0.0 and has_white_up(sheet_segmentado,y,x) and has_white_down(sheet_segmentado,y,x)): 
                        continue
                    # write_point(x,y,i)
                    only_border[i][y][x] = 100
                    found = False
        # f.write("===\n")
    for sheet_id in exists_seg:
        name = str(sheet_id) + ".npy"
        filename=os.path.join(save_path,name)
        np.save(filename, only_border[sheet_id])
        write_ordered(only_border[sheet_id], sheet_id)
    print("Saved files in", save_path)



get_border_segmentation()
f.close()