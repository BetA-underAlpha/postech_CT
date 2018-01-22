#include"client.h"
#include"ai.h"

int main()
{
	/*
	variable information
	- caseNumber [type : int] => 1 : test using k-fold, 2 : run AI
	- nb_class [type : int] => number of class ( apple, bomb, face, background ) : total (4)
	- train_set [type : array(img, true_class_number)] => img is literally image, true_class_number is true class of that image.
	- socketInfo => you don't have to know this variable
	*/

	int caseNumber;
	int nb_class = 4;

	vector<pair<Mat, int> > train_set;
	socketSet socketInfo;

	srand(time(NULL));

	cout << "MENU (1: testing, 2: network)\n" << endl << "> ";

	cin >> caseNumber;

	switch (caseNumber)
	{
	case 1:
		cout << "\n\nLOADING IMAGE...\n";
		for (int i = 0; i < nb_class; i++)
		{
			string path = to_string(i);
			if (!input(train_set, path, i))
				std::cout << "error" << endl;
		}
		cout << "COMPLETE!\n\n";
		cout << "EVALUATING...\n";

		cout << "ACCURACY : " << (1 - model_evaluate(train_set, nb_class)) * 100 << "%" << "\n\n\n";

		break;

	case 2:
		initClient(&socketInfo);
		runClient(&socketInfo);
		endClient(&socketInfo);

		break;
	}

	system("pause");

	return 0;
}