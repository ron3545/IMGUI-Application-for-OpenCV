#include "../Header Files/HunggarianAlgorithm.hpp"
#include <algorithm>


std::vector<size_t> KuhnMunkres::Solve(const cv::Mat& dissimilarity)
{
	CV_Assert(dissimilarity.type() == CV_32F);

	size_n = std::max(dissimilarity.rows, dissimilarity.cols);
	m_dm = cv::Mat(size_n, size_n, CV_32F, cv::Scalar(0));
	m_marked = cv::Mat(size_n, size_n, CV_8S, cv::Scalar(0));
	m_Point = cv::Point(size_n * 2);

	dissimilarity.copyTo(m_dm(cv::Rect(0, 0, dissimilarity.cols, dissimilarity.rows)));
	std::vector<size_t> results(dissimilarity.rows, -1);
	
	ExecutePhases();
	for (const auto& pos : ZeroPos)
		results.push_back(pos.column);

	return results;
}

#pragma region Phase 1
void KuhnMunkres::RowReduction()
{
	auto is_row_visited = std::vector<int>(size_n, 0);
	auto is_colmn_visited = std::vector<int>(size_n, 0);
	for (int row = 0; row <= size_n; row++)
	{
		auto ptr = m_dm.ptr<float>(row); //returns a pointer to a specific  matrix row
		auto marked_ptr = m_marked.ptr<char>(row);
		auto min_val = *std::min_element(ptr, ptr + size_n); // it finds the minimum value althroughout th row

		for (int col = 0; col <= size_n; col++)
		{
			ptr[col] -= min_val;
			if (ptr[col] == 0 && !is_colmn_visited[col] && !is_row_visited[row])
			{
				marked_ptr[col] = KStar; //mark this point as it's containing zero
				is_row_visited[row] = 1;
				is_colmn_visited[col] = 1;
			}
		}
	}
}

void KuhnMunkres::ColmnReduction()
{
	auto is_col_visited = std::vector<int>(size_n, 0);
	auto is_row_visited = std::vector<int>(size_n, 0);
	auto min_values = std::vector<float>(size_n, 0);

	for (int row = 0; row <= size_n; row++)
	{
		auto ptr = m_dm.ptr<float>(row);
		auto min = ptr[0];
		for (int col = 0; col <= size_n; col++)
		{
			if (ptr[col] < min)
			{
				min = ptr[col];
			}
		}
		min_values.push_back(min);
	}

	for (int col = 0; col <= size_n; col++)
	{
		auto min = min_values[col];
		for(int row = 0; row <= size_n; row++)
		{ 
			auto ptr = m_dm.ptr<float>(row);
			auto marked_ptr = m_marked.ptr<char>(row);
			ptr[col] -= min;
			if (ptr[col] == 0 && !is_row_visited[row] && !is_col_visited[col])
			{
				marked_ptr[col] = KStar; //marks this [osition int he matrix if it's value is zero
				is_row_visited[row] = 1;
				is_col_visited[col] = 1;
			}
		}
	}
}
#pragma endregion

void KuhnMunkres::RowScanning()
{
	for (int row = 0; row <= size_n; row++)
	{
		Point pos; // y = row; x = column
		const auto marked_ptr = m_marked.ptr<char>(row );
		int count = 0;//determines that the number of zeros on each row is only limited to one
		
		for (int col = 0; col <= size_n; col++)
		{
			if ( !std::binary_search(skip_colmns.begin(), skip_colmns.end(), col) ) //check if we need to skip this column
			{
				if (marked_ptr[col] == KStar) {
					count++;
					pos.column = col;
					pos.row = row;
				}
			}
		}

		if (count == 1)
		{
			Zeros_at_rows.push_back(pos);
			ZeroPos.push_back(pos);
			skip_colmns.push_back( pos.column );
		}
	}
}

void KuhnMunkres::ColScanning()
{
	
	for (int col = 0; col <= size_n; col++)
	{
		int count = 0;
		Point pos;
		if (!std::binary_search(skip_colmns.begin(), skip_colmns.end(), col))
		{
			for (int row = 0; row <= size_n; row++)
			{
				if (!std::binary_search(skip_row.begin(), skip_row.end(), row))
				{
					const auto marked_ptr = m_marked.ptr<char>(row);
					if (marked_ptr[col] == KStar)
					{
						count++;
						pos.column = col;
						pos.row = row;
					}
				}
			}
			
			if (count == 1)
			{
				Zeros_at_cols.push_back(pos);
				ZeroPos.push_back(pos);
				skip_row.push_back(pos.row);
			}
		}
	}
}

bool KuhnMunkres::IsOptimalFound()
{
	RowScanning();
	ColScanning();
	if (ZeroPos.size() == size_n)
		return true; //because the number of zeros are equal to the size of the matrix

	return false; //optimal haven't found yet
}

void KuhnMunkres::ExecutePhases()
{
	RowReduction();
	ColmnReduction();

	//O(n^2)
	while (!IsOptimalFound())
	{
		//find the intersection points on the matrix
		Position IntersectionPoints;
		for (const auto& posx : Zeros_at_cols)
		{
			for (const auto& posy : Zeros_at_rows)
			{
				Point point;
				point.Set(posy.column, posx.row); //the intersection point
				IntersectionPoints.push_back(point);
			}
		}

#pragma region find minimum undeleted cell values
		int min_undeleted_cell_val = 0;
		std::vector<float> undeleted_cell_val;
		for (int row = 0; row <= size_n; row++)
		{
			if (!std::binary_search(skip_row.begin(), skip_row.end(), row)) {
				auto ptr = m_dm.ptr < float >(row);
				for (int col = 0; col <= size_n; col++)
				{
					if (!std::binary_search(skip_colmns.begin(), skip_colmns.end(), col))
					{
						undeleted_cell_val.push_back(ptr[col]);
					}
				}
			}
		}
		min_undeleted_cell_val = *std::min_element(undeleted_cell_val.begin(), undeleted_cell_val.end());
#pragma endregion

		//add the minimum undeleted cell value to the intersected values
		//step 3a 
		for (const auto& coordinate : IntersectionPoints)
		{
			auto ptr = m_dm.at<float>(coordinate.row, coordinate.column);
			ptr += min_undeleted_cell_val;
		}

		undeleted_cell_val.clear();
		//subtracting the smallest alue among the undeleted cell values
		for (int row = 0; row <= size_n; row++)
		{
			if (!std::binary_search(skip_row.begin(), skip_row.end(), row)) {
				auto ptr = m_dm.ptr < float >(row);
				for (int col = 0; col <= size_n; col++)
				{
					if (!std::binary_search(skip_colmns.begin(), skip_colmns.end(), col))
					{
						ptr -= min_undeleted_cell_val;
					}
				}
			}
		}
	}//end of while loop
}