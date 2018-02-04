#include "pch.h"
#include "Utilities.h"
std::vector<uint16_t> reverseIndices(std::vector<uint16_t> indices) {
	std::vector<uint16_t> reversed;
	for (int i = indices.size() - 1; i >= 0; i--)
		reversed.push_back(indices[i]);
	return reversed;
}