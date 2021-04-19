#include "utils/utils.h"
#include "utils/vcrash.h"
#include "math/Matrix.h"
#include "math/NeuralNetwork.h"
#include "math/math.h"
#include "file/FileReader.h"
#include "file/ImageReader.h"

using namespace vio;

// Mettre ici les fonctions qui testent la bibliothèque.
void test_matrix(){
	debug("test_matrix");
	Matrix a(2,2);
	Matrix b(2,2);
	a.fill(0);
	b.fill(0);

	a.at(0,0) = 2;
	a.at(1,1) = 1;

	b.at(0,1) = -1;
	b.at(1,0) = 1;

	Matrix c = Matrix::mul(a,b);
	vassert(c.get(0,0) == 0 && c.get(1,1) == 0 && c.get(0,1) == -2 && c.get(1,0) == 1);
	
	Vector v = Vector(2);
	v.at(0) = 3;
	v.at(1) = 1;
	Vector v2 = c.apply(v);
	vassert(v2.get(0) == -2 && v2.get(1) == 3)

	Vector v3 = c.applyTranspose(v2);
	vassert(v3.get(0) == 3 && v3.get(1) == 4)

	debug("PASSED.");
}

void test_network(){
	NeuralNetwork nn;
	// add a few layers.
	// Let's build a 3 -> 4 -> 1, aka 2 layers

	// TODO: when providing misshaped layers, an error should happend when evaluating / training ??

	DenseLayer l1(3,4);
	DenseLayer l2(4,1);

	// when initializing, put weights that are similar in scale to the average data.
	l1.randomInit(5);
	l2.randomInit(5);

	nn.layers.push_back(&l1);
	nn.layers.push_back(&l2);


	Vector input(3);
	input.at(0) = 3;
	input.at(1) = 1;
	input.at(2) = 2;

	Vector output = nn.apply(input); // just make sure this does not crash.

	// let's generate some data to train the network !
	std::vector<Vector> trainingInputs;
	std::vector<Vector> trainingOutputs;
	for(u32 i = 0;i < 1000;i++){
		Vector newIn(3);
		Vector newOut(1);

		newIn.at(0) = randomFloat()*20 - 10;
		newIn.at(1) = randomFloat()*20 - 10;
		newIn.at(2) = randomFloat()*20 - 10;

		newOut.at(0) = newIn.get(0) * 3 + newIn.get(1) * 5 + newIn.get(2) * 0;

		trainingInputs.push_back(std::move(newIn));
		trainingOutputs.push_back(std::move(newOut));
	}


	auto begin_timer_clock = std::chrono::high_resolution_clock::now();

	nn.prepare();

	time_function("training the test network",[&nn,&trainingInputs,&trainingOutputs](){
		float rate = 0.001;
		float previousError = 99999;

		for(u32 i = 0;i < 2000;i++){
			nn.train(trainingInputs,trainingOutputs);
			float e = nn.RMSerror(trainingInputs,trainingOutputs);
			if(e < 0.5) break;
			if(e > previousError){
				rate *= 0.99;
			}
			previousError = e;
			//if(i%20 == 0){
			//debug("Error %i = %f, r = %f",i,e,rate);
			//}
		}
	});

	float e = nn.RMSerror(trainingInputs,trainingOutputs);
	vassert(e < 0.5); // error minimized in less than 2000 training steps.

	// Alright, now, let's test our network !

	Vector vresult = nn.apply(trainingInputs[0]);
	vresult -= trainingOutputs[0];

	vassert(vresult.norm() < 5); // on a training example
	output = nn.apply(input); // Let's compute 3*3 + 5*1 - 2*2 ( = 9 + 5 - 4 = 10)
	output.print();
	vassert(abs(output.get(0) - (3*input.get(0) + 5*input.get(1) - 2*input.get(2))) < 5); // on a non-training example.

	debug("PASSED.");
}

void test_file(){
	std::string p = getExecutablePath();

	debug("p = %s",p.c_str());

	ImageReader ir(getExecutableFolderPath() + "/example2.png");

	debug("Size: w,h,alpha %i %i %i",ir.getWidth(),ir.getHeight(),ir.isAlpha());
	debug("Pixel: %i %i %i",ir.pixelAtPos(0,999,0),ir.pixelAtPos(0,999,1),ir.pixelAtPos(0,999,2));


	debug("Printed.");
}


int main(int argc,char ** argv){
	utf8_console();
	setup_crash_handler();
	debug("Starting tests ...");
	test_matrix();
	//test_network();
	test_file();

	debug("All tests passed.");

}
