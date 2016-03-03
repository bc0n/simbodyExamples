// Ben Conrad -- Sliding Block Simbody Example -- 20160303
#include "Simbody.h"

using namespace SimTK;

// A regularly-scheduled event reporter
class PositionReporter : public PeriodicEventReporter {
public:
	//Inheritance accomplished above; below, the colon indicates member function intialization
	PositionReporter(const MultibodySystem& system, const MobilizedBody& mobod, Real reportInterval) : PeriodicEventReporter(reportInterval), system(system), mobod(mobod) {}
	void handleEvent(const State& state) const { //a const-qualified member function...
		system.realize(state, Stage::Position); // need the state's position to be realized to report (recall the state realization flow)
		Vec3 pos = mobod.getBodyOriginLocation(state); //state only knows the gcs, mobod knows assoc. to body
		printf("%10.2f    %10.4f  %10.4f  %10.4f\n", state.getTime(), pos[0], pos[1], pos[2]);
	}
private:
	const MultibodySystem& system;
	const MobilizedBody& mobod;
};

// Regularly writes the bodies' positions to the given file
//  -- apbodies is a pointer to an array of bodies whose positions we want to write to the file
class ArrayPositionWriter : public PeriodicEventReporter {
public:
	ArrayPositionWriter(const MultibodySystem& asystem, const MobilizedBody *apbodies, const int anBodies, Real reportInterval, const char *afname) : PeriodicEventReporter(reportInterval), system(asystem), pbodies(apbodies), nBodies(anBodies) {
		printf("Opening %s\n", afname);
		fs = fopen(afname, "w");
	}
	~ArrayPositionWriter() {
		printf("Closed\n");
		fclose(fs);
	}
	void handleEvent(const State& state) const {
		system.realize(state, Stage::Position); // need the state's position to be realized to report positions (recall the state realization flow)
		std::fprintf(fs, "%f, ", state.getTime());
		Vec3 pos;
		for (int i = 0; i < 1; i++) {
			pos = pbodies[i].getBodyOriginLocation(state);
			std::fprintf(fs, "%f, %f, %f, ", pos[0], pos[1], pos[2]); //the x,y,z position of body[i]
		}
		std::fprintf(fs, "\n");
	}
private:
	const MultibodySystem& system;
	const MobilizedBody *pbodies;
	int nBodies;
	FILE *fs;
};


// Verify dynamic properties:
//  -- single block, mass, damper
//  -- given an initial velocity
//  -- measure the natural frequency and damping ratio from the response (in matlab)
void verifyDynamics() {
	std::cout << "verifyDynamics()\n";
	MultibodySystem system; // Create the system
	SimbodyMatterSubsystem matter(system); //subclass of system, must accompany MultibodySystem
	GeneralForceSubsystem forces(system); //subclass containing a variety of forces

	Body::Rigid block(MassProperties(2.0, Vec3(0), Inertia(1))); //represents physical properties: 1kg, ctr of mass in lrf, inertia
	block.addDecoration(Transform(), DecorativeBrick(Vec3(1))); //a blank cube 1x1x1m
	MobilizedBody::Slider block1(matter.Ground(), Transform(Vec3(10, 0, 0)), block, Transform(Vec3(0))); //x translation only

	//Apply Spring-Damper
	Real springK = 10; //[N/m]
	Real springL = 10; //[m] initial length == starting position
	Real dampC = .1; //[N/m/s]
	Force::TwoPointLinearSpring(forces, matter.Ground(), Vec3(0), block1, Vec3(0), springK, springL); // attach left to ground
	Force::TwoPointLinearDamper(forces, matter.Ground(), Vec3(0), block1, Vec3(0), dampC);

	// Set up visualization
	system.setUseUniformBackground(true); // turn off floor, must come before Visualizer
	Visualizer viz(system);

	// Set up reporters
	system.addEventReporter(new Visualizer::Reporter(viz, 0.2)); //draw a new frame every _sec; viz runs at 30fps=1/30sec
	system.addEventReporter(new PositionReporter(system, block1, .1)); //print a new position every _sec
	system.addEventReporter(new ArrayPositionWriter(system, &block1, 1, 0.1, "verifyDynamics.csv")); //write a new line of block1's position every _sec

	// Initialize the system and state
	system.realizeTopology();
	State state = system.getDefaultState();
	block1.setU(state, 10); //set the block's initial velocity as 10m/s positive x

	// Simulate it
	RungeKuttaMersonIntegrator integ(system);
	TimeStepper ts(system, integ);
	ts.initialize(state);
	ts.stepTo(60.0); //motion nearly finished after 60sec
}

int main() {
	try {//simbody's recommended try/catch
		verifyDynamics();
		std::cout << "Done\n";
	}
	catch (const std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return 1;
	}
}