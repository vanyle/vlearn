#include "utils/utils.h"
#include "utils/vcrash.h"
#include "math/Matrix.h"
#include "math/NeuralNetwork.h"
#include "math/math.h"


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

	SoftReluLayer l1(4,4);
	SoftReluLayer l2(4,1);

	// when initializing, put weights that are similar in scale to the average data.
	l1.randomInit(0,5);
	l2.randomInit(0,5);

	nn.layers.push_back(&l1);
	nn.layers.push_back(&l2);


	Vector input(4);
	input.at(0) = 3;
	input.at(1) = 1;
	input.at(2) = 2;
	input.at(3) = 1;

	Vector output = nn.apply(input);
	debug("Input and output for a 2 layer random network:");
	input.print();
	output.print();

	// let's generate some data to train the network !
	std::vector<Vector> trainingInputs;
	std::vector<Vector> trainingOutputs;

	for(u32 i = 0;i < 1000;i++){
		Vector newIn(4);
		Vector newOut(1);

		newIn.at(0) = randomFloat()*20 - 10;
		newIn.at(1) = randomFloat()*20 - 10;
		newIn.at(2) = randomFloat()*20 - 10;
		newIn.at(3) = 1; // constant term to help the network.

		newOut.at(0) = newIn.get(0) * 3 + newIn.get(1) * 5 + newIn.get(2) * 0;

		trainingInputs.push_back(std::move(newIn));
		trainingOutputs.push_back(std::move(newOut));
	}

	debug("Initial error: %.3f",nn.RMSerror(trainingInputs,trainingOutputs));

	// ToFix: division by zero occur during the gradient descent, they should not.

	auto begin_timer_clock = std::chrono::high_resolution_clock::now();

	nn.prepare();

	time_function("training",[&nn,&trainingInputs,&trainingOutputs](){
		float rate = 0.001;
		float previousError = 99999;

		for(u32 i = 0;i < 3000;i++){
			nn.train(trainingInputs,trainingOutputs);
			float e = nn.RMSerror(trainingInputs,trainingOutputs);
			if(e < 0.7) break;
			if(e > previousError){
				rate *= 0.99;
			}
			previousError = e;
			if(i%20 == 0){
				debug("Error %i = %f, r = %f",i,e,rate);
			}
		}
	});

	// compare for a few elements of the training set how well they are memorized.

	// TODO: why is the error not 0 ???
	float e = nn.RMSerror(trainingInputs,trainingOutputs);
	debug("Final Error (also called loss by some): %.4f",e);

	nn.apply(trainingInputs[0]).print();
	trainingOutputs[0].print();
	debug("---");
	nn.apply(trainingInputs[1]).print();
	trainingOutputs[1].print();
	debug("---");
	nn.apply(trainingInputs[2]).print();
	trainingOutputs[2].print();
	debug("---")

	debug("Content of the layers:")
	nn.layers[0]->print();
	nn.layers[1]->print();

	// Alright, now, let's test our network !
	// Let's compute 3*3 + 5*1 - 2*2 ( = 9 + 5 - 4 = 10)
	output = nn.apply(input);
	output.print();

	debug("PASSED.");
}


int main(int argc,char ** argv){
	setup_crash_handler();
	debug("Starting tests ...");
	test_matrix();
	test_network();

	debug("All tests passed.");

}
