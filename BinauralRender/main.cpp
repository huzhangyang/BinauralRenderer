#include "AudioIO.h"
#include "DataIO.h"
#include <iostream>

int main()
{
	AudioIO *audio = new AudioIO();
	audio->Init();
	audio->Open("c://test.mp3");
	audio->Play();

	DataIO *data = new DataIO();
	auto mat = data->OpenMat("c://test.mat");

	int i, j;
	while (true)
	{
		std::cin >> i >> j;
		auto left = mat->GetLeftHRIR(i, j);
		auto right = mat->GetRightHRIR(i, j);
		if (left != NULL && right != NULL)
		{
			for (int k = 0; k < 200; k++)
			{
				std::cout << left[k] << ", " << right[k] << "\n";
			}
		}
	}
	return 0;
}