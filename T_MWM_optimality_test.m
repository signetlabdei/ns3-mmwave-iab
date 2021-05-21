% Optimality test parameters
clc; clearvars;
iterations = 10^3;
deltaUtil = zeros(1, iterations);
maxChild = 2; maxDepth = 3; maxEdgeWeight = 50;

%-- Functions definitions --%
% Generation of a random, Spanning-Tree structured network
function [edgeList, towardLeaf] = networkGen(maxChild, maxDepth)
counter = 1; prevCounter = 1;
  edgeList = zeros(1000000, maxChild); 
  towardLeaf = zeros(1000000, maxChild); 
  for k = 1:maxDepth
    added = 0;
    for l=prevCounter:counter
      numChildren = randi(maxChild);
      edgeList(l, 1:numChildren) = counter+added+1:counter+added+numChildren;
      if (k == maxDepth)
        towardLeaf(l, 1:numChildren) = ones(1, numChildren);
      endif
      added = added + numChildren;
    endfor
    prevCounter = counter+1;
    counter = counter + added;
  endfor
  edgeList = edgeList(1:counter, :);
  towardLeaf = towardLeaf(1:counter, :);
endfunction

% Convert edge list structure to an OCTAVE compatible one
function [in, out, weights] = matedgeList(edgeList, weightsList)
  numNodes = size(edgeList, 1);
  weights = zeros(1, numNodes*size(edgeList, 2));
  in = zeros(1, numNodes*size(edgeList, 2));
  out = zeros(1, numNodes*size(edgeList, 2));
  added = 1;
  for row = 1:numNodes
    for col = 1:size(edgeList, 2)
      val = edgeList(row, col);
      if (val > 0)
        in(1, added) = row; out(1, added) = val;
        weights(1, added) = weightsList(row, col);
        added = added + 1;
      endif
    endfor
  endfor
  in = in(1, 1:added-1); out = out(1, 1:added-1);
  weights = weights(1, 1:added-1);
endfunction

% Computes random edge weights
function weightsList = computeWeights(edgeList, maxEdgeWeight, towardLeaf)
  activeMask = edgeList > 0;
  randWeights = randi(25, size(edgeList, 1), size(edgeList, 2));
  weightsList = activeMask.*randWeights;
endfunction

% Computes the utility yielded by the T-MWM algorithm described in Sec. III.A
function util = TMWM(edgeList, weightsList)
  fVec = zeros(size(edgeList, 1), 1);
  gVec = zeros(size(edgeList, 1), 1);
  for currNode = size(edgeList, 1):-1:1
    maxVal = -100000; maxInd = -1;
    for targetInd = 1:size(edgeList, 2)
      targetNode = edgeList(currNode, targetInd);
      if(targetNode == 0)
        break;
      else
        gVec(currNode, 1) = gVec(currNode, 1) + max(fVec(targetNode, 1), gVec(targetNode, 1));
        delta = fVec(targetNode, 1) - gVec(targetNode, 1);
        targetUtil = weightsList(currNode, targetInd);
				targetUtil = targetUtil - max(delta, 0);
        if(targetUtil > maxVal)
          maxVal = targetUtil;
          maxInd = targetInd;
        endif
      endif
    endfor
    if(sum(edgeList(currNode, :)) == 0)
      fVec(currNode, 1) = 0;
    else
      fVec(currNode, 1) = gVec(currNode, 1) + maxVal;
      endif
  endfor
  util = max(fVec(1, 1), gVec(1, 1));
endfunction

function util = recEnum_util(in, out, weights, accumUtil)
  if (size(in, 2) == 0)
    util = accumUtil;
  else
    stored = zeros(1, size(in, 2));
	amountStored = 1;
	while (size(in, 2) > 0)
	  if (size(in, 2) != 1)
	    % Remove edges which share a vertex and recurse
        maskIn = (in != in(1, 1) & in != out(1, 1));
        maskOut = (out != in(1, 1) & out != out(1, 1));
        mask = maskIn & maskOut;
        stored(1, amountStored) = recEnum_util (in(mask), out(mask), weights(mask), accumUtil+weights(1, 1));
        amountStored++; 
        % Do not check matchings which contain this edge anymore
        in = in(1, 2:end); out = out(1, 2:end); weights = weights(1, 2:end);
      else
        stored(1, amountStored) = weights + accumUtil;
        in = [];
      endif
    endwhile
    util = max(stored);
  endif
endfunction

%%-- T-MWM optimality test --%%
% In each iteration, a random network is generated, and the utilityies obtained with the T-MWM
% algorithm and via exhaustive enumeration are compared.
h = waitbar(0);
for k=1:iterations
  waitbar(k/iterations, h, 'Comparing the MWMs obtained by T-MWM and by global enumeration');
  [edgeList, towardLeaf] = networkGen(maxChild, maxDepth);
  weightsList = computeWeights(edgeList, maxEdgeWeight, towardLeaf);
  [in, out, weights] = matedgeList(edgeList, weightsList);
  utilTMWM = TMWM(edgeList, weightsList);
  utilMax = recEnum_util(in, out, weights, 0);
  deltaUtils(1, k) = utilTMWM - utilMax;
endfor
if (sum(deltaUtils != 0))
  waitbar (1, h, 'The T-MWM algorithm does not compute a global optimum!');
else
  waitbar (1, h, 'The T-MWM algorithm computes a global optimum!');
endif