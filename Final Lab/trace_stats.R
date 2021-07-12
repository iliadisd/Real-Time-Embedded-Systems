#Statistical Analysis for Covid trace.
#Author : Dimosthenis Iliadis-Apostolidis

#Set as working directory the one that the trace file is located. 
setwd("C:/Users/user/Desktop")

#Read the trace data.
trace = read.csv("trace.txt", header = FALSE)

#How many BTnearme() (mac_addresses) did we get?
num_rows = length(trace[, 2])

#Note that we didn't accually get all the measurements we needed for the experiment.
#Running it for "30 days" (7hours and 12 seconds) means that we should have 259200 measurements.
#Instead, in "30 days" we got 241829 measurements.
#This means that we "lost" due to the delay, 17351 measurements which in reality is :

lost = 259200 - num_rows
loss_rate = lost/259200 

#6% loss_rate in this experiment.
#We could say that this simulation has 94% accuracy, on searching for mac_addresses.
#This means that 6% of bluetooth near us might not be found in these requirements, due to the delay.
#We still have 100% accuracy on deleting, uploading and performing the tasks required.

#Initialize a RT_delay vector. R is one-based, but in reality we have 0 -> (num_rows - 1) elements.
#This is still num_rows elements.
RT_delay = matrix(0:0, nrow = num_rows, ncol = 1)

#Calculate the delay table, for each timestamp.
#I just subtract the real timestamp from the one we expected it to run.  [(V1) * 0.1]. 
for (i in 1:num_rows){
  RT_delay[i] = trace[i, 2] - trace[i , 1] * 0.1 
}

#Observe that this delay table includes in each step the delay of the previous one.
#This is reasonable, since the (n+1)th delay includes n (previous) delays.
#Delay grows exponentially over time.
#It also means that the last element is the total delay we had to produce and fetch mac addresses.
total_delay = RT_delay[num_rows]
#We also observe that lost measurements is a number somewhat proportional to the total_delay.

#The above gives us the "real-time" delay for each timestamp
#The CPU delay, though, for each timestamp isn't the above.
#We have to subtract the previous "real-time" delay every time to get the CPU delay that occurs for each fetch.

#Initialize a CPU_delay vector.
CPU_delay = matrix(0:0, nrow = num_rows, ncol = 1)

#Calculate the CPU_delay.
CPU_delay[1] = RT_delay[1]
for (i in 2:num_rows){
  CPU_delay[i] = trace[i, 2] - trace[i , 1] * 0.1 - RT_delay[i-1]
}
#We expect CPU_delay to be different for every BTnearme() call and higher towards the end, cause of parallel execution.

#Now we get the mean value of the delays.
mean_CPU_delay = mean(CPU_delay)
mean_RT_delay = mean(RT_delay)

#Comparing the two values above, we can see that mean_CPU_delay much much lower than the "real-time" delay.
#mean_RT_delay isn't the most important metric. Optimizing CPU_delay is much more important and affects "real-time" delay.
#This is reasonable as we said that the real-time is all the CPU_delay added in a series.
#In fact if we sum all the CPU_delay values we get the total_delay.
total_CPU_delay = colSums(CPU_delay)

#We plot the RT_delay.
plot(RT_delay, xlab = "Timestamps", ylab = "RT_delay")
lines(mean_RT_delay, type="o", pch=22, lty=2, col="red")
#The red square is the mean_RT_delay.
#This confirms the exponential growth of RT_delay over time.
#Clear plot
if(!is.null(dev.list())) dev.off()

#We plot the CPU_delay.
plot(CPU_delay, xlab = "Timestamps", ylab = "CPU_delay")
lines(mean_CPU_delay, type="o", pch=22, lty=2, col="red")
#The red square is the mean_CPU_delay.
#As we can see, towards the end, it takes more time for a mac address to be fetched, due to the execution of many functions.
#e.g. d14 doesn't run until half of the time (day 14 out of 30).
#Clear plot
if(!is.null(dev.list())) dev.off()

#We plot the expected time, as well as the one we got with the RTdelay.
plot(trace[ , 1] * 0.1, type="o", col="blue", xlab = "Timestamps", ylab = "RT_delay")
lines(trace[, 2], type="o", pch=22, lty=2, col="red")
#This shows that RT_delay is growing over time, but still is neglectable. 
#Trying to make these two lines identical is ideal, but cannot be done.

#Clear plot
if(!is.null(dev.list())) dev.off()

#Last but not least, CPU_idle_t.
#Since every moment defined in this exercise is multiple to sec10 (=0.1), the CPU will be idle in this manner:
#For the first "min4" (=2.4), the only function running is s10 and BTnearme() which is called every sec10.
#So the CPU is idle for 2.4 seconds, since the delay in these min4 is the CPU_load_t.
#After that, CPU is idle only for a small amount of time x when sleeping in s10 and nothing else runs. 
#This x could be measured with a timer running in this scenario, but in this case it could be neglectable. 
#So the real answer should be CPU_idle_t = 2.4 + x

#Tested with R 4.1.0
