clc;
clear;
close all;

%% --- 1. Load sparse matrix and ensure undirected structure ---
filename = 'data/dictionary28.mat';
data = load(filename);
A_orig = data.Problem.A;

% *** Correction 1: Ensure A represents an undirected graph (symmetric structure) ***
% The connected components algorithm requires A to be treated as an adjacency matrix 
% of an UNDIRECTED graph. We use the union of A and A's structure.
A = A_orig + A_orig';
A(logical(speye(size(A)))) = 0; % Remove self-loops (optional, but standard)
A = spones(A); % Only non-zero structure matters for connectivity

[n, m] = size(A); % n should equal m for typical adjacency matrices

%% --- 2. Convert to CSC format vectors (Corrected Method) ---
[row_idx, col_idx, ~] = find(A); % We only need row_idx and col_idx, not val

% Convert to int32 for performance and compatibility (good practice)
row_idx = int32(row_idx);
col_idx = int32(col_idx);
nnz_count = length(col_idx);

% *** Correction 2: Correctly build col_ptr using cumulative sum ***
% This is the standard, robust way to generate CSC pointers.

% 2a. Count non-zero elements in each column (histogram)
col_counts = zeros(m, 1, 'int32');
for k = 1:nnz_count
    col_counts(col_idx(k)) = col_counts(col_idx(k)) + 1;
end

% 2b. Calculate col_ptr using cumulative sum
% col_ptr(j) stores 1 + the number of non-zeros in columns 1 to j-1.
col_ptr = [int32(1); int32(1) + cumsum(col_counts)];


%% --- 3. Initialize labels ---
% Initialize each node (row index u) with its own unique label
label = int32(1:n)';
changed = true;

%% --- 4. Min-copy propagation (The actual algorithm) ---
% Note: The row index u in row_idx corresponds to the original node, 
% and the column index j corresponds to the adjacent node.
% Since A is now symmetric, the row/column distinction is less critical for connectivity, 
% but we strictly follow the sparse format iteration.

while changed
    % Copy current labels to check for changes in this iteration
    label_new = label;
    changed = false;

    % Iterate through columns (nodes v)
    for j = 1:m % j is the column index (node v)
        col_start = col_ptr(j);
        col_end   = col_ptr(j+1) - 1;
        
        if col_end < col_start
            continue; % Empty column
        end
        
        % Rows contains the indices (nodes u) connected to node j (v)
        rows = row_idx(col_start:col_end);
        
        % Find the smallest label among node v (j) and all connected nodes u
        current_labels = [label(j); label(rows)];
        min_label = min(current_labels);
        
        % Propagate the minimum label to the current node v (j)
        if label_new(j) > min_label
            label_new(j) = min_label;
            changed = true;
        end
        
        % Propagate the minimum label to all connected nodes u (rows)
        % Note: Using vector operations for efficiency here
        idx_to_update = rows(label_new(rows) > min_label);
        if ~isempty(idx_to_update)
            label_new(idx_to_update) = min_label;
            changed = true;
        end
    end

    % Update labels for the next iteration
    label = label_new;
end

%% --- 5. Output number of connected components ---
components = unique(label);
num_components = length(components);
fprintf('Number of connected components: %d\n', num_components);