% extend the image by 1 pixel using sparse coding method
% - A: dictionary
% - overlap_A_left: dictionary when consider only a left region as roi
% - size: size of the patch
function img = extend_img_sparsecoding(input_img, A, roi_A_left, patch_size)
    addpath('CVPR08SR/Sparse Coding');
    
    d = size(input_img);
    width = d(2);
    height = d(1);
    channel = d(3);
    
    % new patches
    result = zeros (patch_size * patch_size * channel, height);
    index = 1;
    % for each patch of size x (size - 1) in the left
    for y = 1:patch_size:height - patch_size + 1  
        patch = input_img(y:1:y + patch_size - 1, (width - (patch_size-1) + 1):1:width, 1:1:channel);
        patch_col = col_convert(patch);        
        coeff = sparse_coding2(roi_A_left, patch_col, 0.1, []);
        % coeff = SolveLasso(roi_A_left, patch_col, size(roi_A_left, 2),'nnlasso', [], 100); 
        result(:, index) = A * coeff;   
        
%         patch_new = col_to_matrix(result(:, index), patch_size - 1, patch_size, channel);         
%         patch_new = uint8(patch_new);
%         figure(1);        
%         imshow(patch_new);        
%         figure(2);
%         patch = uint8(patch);
%         imshow(patch);
        
        index = index + patch_size;         
    end
    
    % apply new patches to the image, result in a new image        
    img = ones(height, width + 1, channel);
    % old region is the same
    img(1:1:height, 1:1:width, 1:1:channel) = input_img;
    for y = 1:patch_size:height - patch_size + 1
        patch = col_to_matrix(result(:, y), patch_size, patch_size, channel);
        % only take the last column
        % img(y:1:y + patch_size - 1, width + 1, 1:1:channel) = patch(1:1:patch_size, patch_size, 1:1:channel);
        img(y:1:y + patch_size - 1, width - patch_size + 2:1:width + 1, 1:1:channel) = patch(1:1:patch_size, 1:1:patch_size, 1:1:channel);
    end      
return;