% dense sampling patches from an image
function A = dense_sampling(img, patch_size, patch_increment)
d = size(img);
width = d(2);
height = d(1);
channel = d(3);

% size of the patch, each channel will be align and merge into 1 column
patch_total = patch_size * patch_size * channel;
% number of patches in the image
patch_count = floor((width - patch_size + 2)/patch_increment) * floor((height - patch_size + 2)/patch_increment);

A = ones(patch_total, patch_count);
index = 1;

fprintf('start sampling....');
for i = (1:patch_increment:width - patch_size + 1)
    for j = (1:patch_increment:height - patch_size + 1)
        % extract patch        
        patch = img(j:1:j + patch_size - 1, i:1:i + patch_size - 1, 1:1:channel);
        % convert patch to 1 col matrix
        patch_col = col_convert(patch);        
        % assign col to A         
        A(:, index) = patch_col;       
        index = index + 1;         
    end     
end
fprintf('end sampling\n');
return;