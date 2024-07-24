
# STM32_SCREEN_LTDC_DCMI_OV7670
Bare metal project to extract output from an OV7670 camera at higher speed (10 fps for SPI, 30 fps for LTDC). This is the third project in a sequence of projects.  

## General description
In the second project, we were extracting images from the OV7670 at roughly 2 fps. This was due to various limitations that we imposed on ourselves to simply the project and get an output on the screen. It was thus obvious from the start that this is not the maximum we can achieve with the OV7670+DISCO setup. Not to mention, there already are multiple examples of similar project on the internet that show that 10 fps could be achieved without an issue should we choose to be more on-point with our timings…

…and yet, even after achieving that particular outcome (see below in the section "Climb to 10"), I still was convinced that we could do even better.

Let’s see what I ended up cooking up!

### Recap
To give a recap over the architecture (with clocking indicated):

1) We generate images with the OV7670 camera at a speed defined by the internal clock of the camera (which is powered by an external master clock).
2) We use the DCMI interface to capture the image output where the DCMI is clocked by the PIXCLK output of the camera.
3) We transfer the captured image from the DCMI peripheral to the frame buffer in chunks of 32 bits, using a DMA, clocked by the MCU's own AHB1 clock.
4) We publish the frame buffer by using the SPI+DMA combination (SPI clocked by APB2, DMA by AHB1) or, alternatively, by using something called the LTDC (also on APB2).
5) The ILI9341 stores the frame buffer in its GRAM and the publishes the image according to its own internal clock (between 60 fps to 120 fps, depending on init).

Technically, we transfer the image through multiple buffers using multiple peripherals that have their own clocks and limitations. What may be a bit difficult to get is that we have an interplay between these peripherals and drivers that can, to some extent, offer us potential workarounds given they are properly set up. All in all, we might be able to simplify the problem to a pure timing challenge. We do need to profoundly understand, what is going on within these peripherals first though by reading the documentation and doing a lot of trial and error. OF note, I will not be touching upon the trial and error phases I had to go through to achieve the outcome.

## Need for speed 
Looking at the meagre 2 fps we had in the second project (and the proven 10 fps potentially available to us), it is clear that we need to squeeze out more speed from the peripherals to attain greater framerate as well as decreased the timing delay that currently exists between the engagement of these peripherals. We have a few tools at our disposition to achieve that.

### Interrupts
Every capability we are using in our setup has its own set of interrupts. This means that, given the triggers are properly set and managed, we can completely drop the crude delay functions we were relying upon.

Crude delays can still be pretty useful for debugging though. For instance, we can track the execution speed of a peripheral without an oscilloscope by adding a break point into the IRQ handler and then step out of it using the debug tool. Wherever we will land in the delay functions execution cycle, will be a rough estimate of how long it takes for the peripheral to reach its IRQ trigger.

### MCU clocking
In the second project, we ended up clocking our MCU at 60 MHz for the simple reason that generating a 12 MHz master clock for the OV7670 was easy from that value. Unfortunately, that also meant that we were clocking all AHB/APB busses 3 times slower than what could be achieved. Increasing the MCU’s clocking should mean that all peripherals' execution (and, by proxy, the data flow) should increase. I ended up going with a clocking of 120 MHz.

### Camera clocking
We are clocking the camera with a 12 MHz master clock (provided by PWM from the MCU), which – with the original camera init matrix from Adafruit – results in a camera capture frame rate of 30 fps. The datasheet of the camera does suggest though that at 320x240 resolution, we should be able reach 60 fps, a two-fold increase.

Indeed, we can attain this output speed by using the camera’s own PLL to multiple the master clock. Originally, we weren't using the PLL.

Mind, we are controlling the DCMI using the camera’s outputs, so increased clocking on the camera will lead to increased image capture to the frame buffer...and even more noise.

### Screen driver clocking
We can also increase the refresh rate of the screen by manipulating the ILI’s own clocking and refresh rate. Since this refresh rate is related to whatever is in the GRAM, we don't really need to bother ourselves with it. We just need to keep in mind that if the ILI input is not clocking (SPI+DMA or LTDC), the screen won't be clocking either.

### LTDC
Lastly, we can use a completely new peripheral (LTDC) to extract data from the frame buffer. It is a parallel interface that – in theory – should allow us to circumvent the problems we were facing using SPI+DMA since, i.e. the transfer limit and low speed. Mind, the LTDC provides an “RGB interface”, thus we will need to readjust the ILI screen driver to make use of it.

## Previous relevant projects
We will build on the previous project in the line:
- STM32_SCREEN_SPI_DCMI_OV7670 

## To read
There is no new reading material for this project. Profound understanding of what the peripherals and drivers are doing is pivotal though, so keep the datasheet close.

## Particularities
We were using crude delays to time the output in the second project, literally blocking all code execution until we were roughly sure that the image has been transferred. It worked well to get some kind of output done, but made the timing of each step indicated in the “Recap” section completely haphazard. It was obvious that we were losing a lot of time, something that we could regain by dropping - or at least, decreasing - the delays. Let's see. what could be done!

### Climb to 10
If we set up the speed tracking I have indicated above (using proper IRQ triggers and delays after function calls), it will be clear that it takes us roughly 250 ms to store an incoming image in the frame buffer and about 50 ms to publish one section of the image to the screen within the second project. This actually aligns well with our experience that the frame rate of the second project is around 2 fps.

If we clock the MCU up to 180 MHz, we will immediately have a faster clocking of all peripherals and thus decrease capture time. At the same time, if we change the 0x6b register’s value in the OV7670 from 0x0 to 0x40, we will start clocking the camera using its own x4 multiplier PLL. These two changes will already drop the time to capture from 250 ms to around 30 ms (exact numbers are hard to tell due to the capture start date not necessarily overlapping with the camera’s own start state) and increase publishing speed from 50 ms/section to 25 ms/section. With all this, we already are publishing one frame every 130 ms, a value bit above 9 fps. Mind, these timings can be even further improved by dropping the delays completely and using blocking “while” loops with an IRQ-set flag to time the execution of the code. (I have implemented such trigger on the input side only side since my focus was squarely on the LTDC instead.)

### LTDC-ephant in the room
There are multiple problems with the LTDC. We will first need to carve the flow in a manner that we will have a properly aligned output and then, we need to get around the physical limitations before the timing could be repaired. This section took a lot of trial and error to make.

#### 320x240 vs 240x320…or 240x240
We were already playing around with the data flow a bit, rotating the image when it was necessary. Nevertheless, once we look into it, we are very much limited in what we can do with the data flow with the LTDC activated.

Now, the LTDC will ALLWAYS read along the 320 pixel axis and we cannot change that easily (or at all, it was a bit too tedious of a work to investigate this to the end). This means that it will read 240 pixels first and then go to the next line. Thus, we MUST load the data to the frame buffer in 240 pixel chunks for the LTDC to properly place them on the screen. In other words, we need to have an image that is 240x320 instead of 320x240…

This is at odds with how we are capturing the incoming camera data, loading it into a 1-dimensional frame buffer: in case of QVGA resolution, the camera will ALLWAYS send over 320 pixels before it would go to the next line. This can’t be changed, we can’t tell the camera to send a column first, not a row. We also can’t change the resolution of the camera from any of the standards (VGA, QVGA or CIF).

In other words, if we use the OV7670 and the LTDC the same time, we will be writing data to the frame buffer in a different direction that the way we extract it, leading to a messed up image. 

Luckily, we have a work around: the DCMI has a “crop” option. We can simply crop the 320x240 incoming image from the camera to 240x240 instead, making it not important, which direction we will be reading the frame buffer out. This will make the data flow compatible with the LTDC by implementing a 25% loss of data. (Of note, extensive play with the LTDC config values likely would allow a proper 320x240 data flow eventually, but I deemed a slight loss of data as acceptable.)

#### Overlapping pins
What immediately became clear to me once I looked at the LTDC the first time was that it was not supposed to work with the DCMI. CubeMX refuses to initialize them both at the same time on an F429ZI and we can do nothing about that using HAL. Luckily, I have a rather hateful relationship with HAL...

Going into the issue of this block, I found out that the DCMI_PIXCLK overlaps with the LTDC_G2 (PA6 pin) and the DCMI_HSYNCH overlaps with the LTDC_VSYNCH (PA4 pin). On the F429ZI, these all are unique pins meaning that we can’t find an alternative to them. To make things worse, the DCMI pins are inputs for the MCU, while the LTDC ones are outputs. We simply can not set them up at the same time and figure out on the fly – within software – what we want to do with them.

No matter, I thought. What we can do is run the DCMI first, capture an image and then shut it off so the pins would be released for the LTDC to use. Unfortunately, while this approach sounds possible, in reality some form of bug blocks the LTDC from releasing the pins after image publishing is done and no matter what I tried, the PA4 and PA6 pins remained locked to the LTDC. As such, we won't be able to use the LTDC_G2 and the LTDC_VSYNCH pins for the LTDC if we want to use the DCMI within our code.

But what does PA4 and PA6 definition left blank mean to the LTDC? Well, losing the LTDC_G2 means that we won’t have the LSB of the green colour, resulting in a type of RGB555 output instead of RGB565. Not a big deal. On the other hand, losing the LTDC_VSYNCH is a big deal since that is the output that indicates the end of a frame. Without it, we won’t be able to tell the ILI9341 when a frame has been stored in its GRAM.

Also, just to rub salt on this injury, even if we don’t define the GPIOs for the LTDC, there still is an input arriving on these two lines to the ILI9341 from the camera since the ILI is physically attached to PA4 and PA6 pins. So the ILI will be triggered on the DCMI’s HSYNCH even without the LTDC properly set up. That's bad. We don't want that.

#### Whack-a-frame
If it hasn’t been clear from the description above, we have three major problems:

1)	We can’t have the DCMI run at the same time as the LTDC
2)	We need to stop the camera when we aren’t “using” it so it won't "bleed" a fake VSYNCH to the ILI9341
3)	We don’t have a way to properly generate the LTDC_VSYNCH signal

Let’s tackle them one by one…
1)	In order to have perceive a smooth image, we need to have at least 30 fps. We know from our SPI speed-up adventure that the camera can provide images at 60 fps and this refresh rate can easily be handled by the DCMI, meaning that on the input side (that is, transfer to the frame buffer) we can have 60 fps without a problem. Now, if we can have the LTDC do its thing in 60 fps (or faster) as well, then we can do a capture to frame buffer, turn off the input side completely, do the publish to the screen and then turn off the output completely, finishing with the entire transfer under 16 ms. 16 ms would be fast enough to provide a smooth image, theoretically. (In reality, we can’t clock the ILI slower than 60 fps so we will have half of the time the screen of, resulting in a barely perceivable flicker.)
2)	The camera can be stopped by depowering it or by stopping its master clock or by putting it into idle mode using its PWDN pin. Depowering isn’t good since it wipes the camera’s init, meaning that we need to reinitialise it upon reactivation, losing time. (Mind, the time while we capture is the time while the screen is off, thus the least time we send in this state, the less of a flicker we will have on the output.) Alternatively, we can stop the master clock and reengage it later. I have tested this option extensively since it is convenient to throw a wrench into the PWM timer, but results have been lacking at best. My guess is that stopping the master clock is not necessarily an immediate action, thus the camera may still tick over once or twice before it stops, messing up the timing for the ILI9341. Finally, I tried the (active LOW) PWDN pin on the OV7670 to put it periodically to idle when we aren’t using it. This ended up working like a charm: once de-activated (PWDN is active LOW), the camera immediately stops and in a recoverable fashion.
3)	We need to indicate the end of a frame without the LTDC_VSYNCH. We have multiple control signals for the LTDC to tell the ILI9341 when an active frame is on the bus. Such signals are the DE and the CLK. If these inputs stop coming in, the ILI9341 will immediately interpret that there is no more active frame data on the RGB interface bus. Luckily, the ILI9341 is smart enough to not freeze but publish whatever is within its GRAM, be that simply a fraction of a frame. As such, if we can kill the LTDC after a certain number of lines it has published, the ILI9341 will publish merrily this “whacked” frame.

#### Interrupts and timing
We are using multiple interrupts to time the execution properly. On the input side, we will need to use both the DCMI’s interrupt (the one that triggers once a frame has been received) and the DMA’s interrupt (the one that triggers on transfer complete). Even though intuitively I would have used only the DMA transfer complete trigger there, that wasn’t working: the DMA TC triggers multiple times before the DCMI’s frame capture for some reason. We ought to carry on with the code only after both the DCMI frame and the DMA TC interrupt have been triggered.

Within the LTDC, I used the "line trigger" interrupt. It tracks how many lines LTDC has published, making it an ideal candidate to solve problem 3). Activating it after 327 lines (320 plus the porches) and stopping the LTDC in the IRQ handler function forces the ILI9341 to stop listening and publish whatever is within its GRAM at that moment...which, due to the line trigger timing we have selected, should be exactly 320x240 pixels. (Mind, a slight speed increase can be attained if we decide to stop the LTDC after only 240 lines. The only penalty is that in this case we won’t have black but “blank” white pixels on the screen where we don't have frame data.)

#### Bugs
Despite all my efforts to make this a robust solution, I think I went just way too deep in the “not allowed” section to make it so. So we have some bugs.

As mentioned above, we have the DMA TC triggering more than necessary.

We also have another bug where the PWDN of the camera must be manually reset after init. I am not sure, what this bug is about, I have tried to put a timeout on it, but, unless we do the reset manually, it doesn’t seem to work. I don’t know if this is a CMOS level problem (CMOS pins should be pulled LOW when not used, something my adaptor board doesn’t do right now) or could be a timing problem (where the reset has to occur at a certain point in the execution).

Lastly, noise is getting even greater of an issue here due to the increased speed. Without an adapter board, this project did not work for me whatsoever.


## User guide
I have left the 10 fps SPI+DMA version in the code behind #ifdef markers. It works without any extra user attention.

For the 30 fps LTDC version to work, one merely needs to power up the setup and then manually reset the camera using the PWDN pin (just ground it with a wire).

## Conclusion
I am rather proud of this project since I have not seen anyone churn out 30 fps from a F429_DISCO before. Nevertheless, if I make a follow-up project, I likely will use something more powerful. We are very much at the limits of what we can do with this setup.
