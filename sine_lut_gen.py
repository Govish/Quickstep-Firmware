import numpy as np
import matplotlib.pyplot as plt
import pyperclip

NUM_POINTS = 4096
FLOAT_PRECISION = 6

x = np.arange(NUM_POINTS)
sin_x = np.sin((2 * np.pi / 4096.0) * x)

plt.plot(x, sin_x)
plt.show()

np.set_printoptions(
	precision = 6,
	threshold = NUM_POINTS,
	linewidth = 150
	)

print(sin_x)
print(np.size(sin_x))
pyperclip.copy(np.array2string(sin_x, separator=", "))

