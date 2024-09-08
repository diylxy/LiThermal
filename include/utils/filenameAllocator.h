#pragma once

int getTotalImages();

int getLastImageID();

int getNextImage(int current);

int getPrevImage(int current);

void readFiles(const char *dirToOpen);

const char *allocateNewFilename();
