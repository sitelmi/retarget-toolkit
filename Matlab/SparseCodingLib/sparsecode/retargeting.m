addpath('solver');
addpath('./l1magic');
addpath('./spgl1-1.6');
img = imread('lion.jpg');
d = size(img);
width = d(2);
height = d(1);
channel = d(3);
A = random_sampling(img, 8, 2);
 
% result_new = A(:, 200);
% patch_new = col_to_matrix(result_new, 8, 8, channel);         
% patch_new = uint8(patch_new);
% imshow(patch_new);

% overlap_left_1 = roi_matrix_left(8, channel, 7);
mask_matrix = ones(8,8,3);
for ch = 1:1:3
    mask_matrix(:,:,ch) = [ 
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;
        1 1 1 1 1 1 1 0;        
        ];
end
overlap_left_1 = extraction_matrix(mask_matrix);            

overlap_left_A = overlap_left_1 * A; 
% new_img = extend_img_sparsecoding(img, A, overlap_left_A, 8);
new_img = extend_img_sparsecoding2(img, A, 8, 4);
new_img = uint8(new_img);
imshow(new_img);
for i = 1:1:10
    new_img = extend_img_sparsecoding2(new_img, A, 8, 4);
    new_img = uint8(new_img);
    imshow(new_img);
end