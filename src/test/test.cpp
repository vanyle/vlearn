#include "file/File.h"
#include "utils/utils.h"
#include "utils/vcrash.h"
#include "math/Matrix.h"
#include "math/NeuralNetwork.h"
#include "math/math.h"
#include "file/ImageReader.h"

#include <unistd.h>

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
			float e = nn.loss(trainingInputs,trainingOutputs);
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

	float e = nn.loss(trainingInputs,trainingOutputs);
	vassert(e < 0.5); // error minimized in less than 2000 training steps.

	// Alright, now, let's test our network !

	Vector vresult = nn.apply(trainingInputs[0]);
	vresult -= trainingOutputs[0];

	vassert(vresult.norm() < 5); // on a training example
	output = nn.apply(input); // Let's compute 3*3 + 5*1 - 2*2 ( = 9 + 5 - 4 = 10)
	output.print();
	vassert(abs(output.get(0) - (3*input.get(0) + 5*input.get(1) - 2*input.get(2))) < 5); // on a non-training example.

	// Now what the training as tested, let's verify that the Conv layer works.
	// Here are some training example from stack exchange.

	// input: [[2, 9, 3, 8], [0, 1, 5, 5], [5, 7, 2, 6], [8, 8, 3, 6]]
	// convolution: [[9,8], [8,6]]
	// reduction factor: 2
	// expected output:

	debug("PASSED.");
}

void test_file(){
	std::string p = getExecutableFolderPath();
	ImageReader ir(getExecutableFolderPath() + "/example2.png");
	for(u32 x = 0;x < ir.getWidth();x++){
		for(u32 y = 0;y < min(30,ir.getHeight());y++){
			ir.setPixelAtPos(x,y,0,255);
		}
	}
	ir.save(p+"/edited.png");
	debug("PASSED.");
}

u32 getBytesAsInt(File& f){
	u32 r = f.nextChar()<<24;
	r += f.nextChar()<<16;
	r += f.nextChar()<<8;
	r += f.nextChar();
	return r;
}

void test_mnist(){
	// a complete test of file reading and neural network: digit recognition

	File file_images(getExecutableFolderPath() + "/training_data/train-images.idx3-ubyte");
	File file_labels(getExecutableFolderPath() + "/training_data/train-labels.idx1-ubyte");

	// the first 4 bytes of each file are:
	// 0x00000801 (magic number)
	// after that there are 4 bytes for the number of items

	// inside images, after that, there is the size of width of the images
	// than, there are the images, with 1 byte = 1 pixel (black+white, row-wise) and for the labels, 1 byte = 1 label

	vassert(getBytesAsInt(file_images) == 0x803);
	vassert(getBytesAsInt(file_labels) == 0x801);
	const u32 item_count = getBytesAsInt(file_images);
	vassert(item_count == getBytesAsInt(file_labels));
	vassert(item_count > 0);

	const u32 image_width = getBytesAsInt(file_images);
	const u32 image_height = getBytesAsInt(file_images);

	vassert(image_width == image_height && image_width > 0 && image_width < 300);

	// alright, now we can access the training data !
	std::vector<Vector> labels;
	std::vector<Vector> images;

	debug("Image file size: %i",file_images.size());

	for(u32 i = 0;i < item_count;i++){
		Vector label(10); // one for every digit
		label.fill(0);

		int c = file_labels.nextChar();
		label.at(c) = 1;
		labels.push_back(label);

		Vector image(image_width * image_height);
		for(u32 j = 0;j < image_width * image_height;j++){
			int pixel = file_images.nextChar();
			image.at(j) = (float)(pixel) / 255.;
		}
		images.push_back(image);

	}
	debug("Training data loaded: %i items, image size: %i x %i",item_count,image_width,image_width);

	NeuralNetwork nn;
	ConvLayer cl1(image_width*image_height,2,8,8); cl1.randomInit(2,1);
	ConvLayer cl2(image_width*image_height / 4,2,8,8); cl2.randomInit(2,1);
	DenseLayer l3(image_width*image_height / 16,30); l3.randomInit(3); // size reducted to "only" 49 parameters
	DenseLayer l4(30,10); l4.randomInit(3);
	SoftMaxLayer l5(10);

	nn.layers.push_back(&cl1);
	nn.layers.push_back(&cl2);
	nn.layers.push_back(&l3);
	nn.layers.push_back(&l4);
	//nn.layers.push_back(&l5);
	nn.prepare();

	debug("Loss: %.6f",nn.loss(images,labels));

	for(u32 i = 0;i < 100;i++){
		nn.train(images,labels,0.001);
		debug("Loss: %.6f",nn.loss(images,labels));
		//nn.layers[0]->print();
		usleep(1000 * 500);
	}

	debug("Loss: %.6f",nn.loss(images,labels));


	debug("PASSED.");

}
void test_http(){
	// let's try to leak memory !
	constexpr u32 rounds = 1'000'000;


}


int main(int argc,char ** argv){
	utf8_console();
	setup_crash_handler();
	debug("Starting tests ...");
	test_matrix();
	//test_network();
	//test_file();
	test_mnist();
	//test_http();

	debug("All tests passed.");

}
