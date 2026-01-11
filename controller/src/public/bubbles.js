const canvas = document.getElementById("bubbleCanvas");
const context = canvas.getContext("2d");

canvas.width = window.innerWidth;
canvas.height = window.innerHeight;

const maxRadius = 30;       // max bubble radius
const minSecsTween = 2;     // minimum time between bubbles
const maxSecsTween = 8;    // maximum time between bubbles


var particleArray = [];
class Particle {
	constructor(x = 0, y = 0) {
	    this.x = x;
	    this.y = y;
	    this.radius = Math.random() * maxRadius;
	    this.dx = 0;
	    this.dy = Math.random() * 3 + 2;
	    this.hue = 200;
	}

	//draw circle
	draw() {
		context.beginPath();
		context.arc(this.x, this.y, this.radius, 0, 2 * Math.PI);
		context.strokeStyle = `hsl(${this.hue} 100% 50%)`;
		context.stroke();

		const gradient = context.createRadialGradient(
			this.x,
			this.y,
			1,
			this.x + 0.5,
			this.y + 0.5,
			this.radius
		);

		gradient.addColorStop(0.3, "rgba(255, 255, 255, 0.3)");
		gradient.addColorStop(0.95, "#e7feff");

		context.fillStyle = gradient;
		context.fill();
	}

	// move circle
	move() {
		this.x = this.x + this.dx;
		this.y = this.y - this.dy;
	}
}

const handleDrawCircle = (horizPos) => {
    if(particleArray.length < 10) {
	const particle = new Particle(horizPos, window.innerHeight + 20);
	particleArray.push(particle);
    }
};

const animate = () => {
    context.clearRect(0, 0, canvas.width, canvas.height);

    particleArray.forEach((particle) => {
	particle?.move();
	particle?.draw();
    });

    particleArray = particleArray.filter((particle) => particle.y > -100);

    requestAnimationFrame(animate);
};

animate();

function randomBubble()
{
    var position = Math.random() * window.innerWidth;
    var newTimeout = (Math.random() * (maxSecsTween - minSecsTween) + minSecsTween) * 1000;

//    console.log("Random Bubble",position,newTimeout);
    handleDrawCircle(position);
    setTimeout(randomBubble,newTimeout);
}
    

randomBubble();

// canvas.addEventListener("click", handleDrawCircle);
// canvas.addEventListener("resize", () => {
//	canvas.width = window.innerWidth;
//	canvas.height = window.innerHeight;
// });
