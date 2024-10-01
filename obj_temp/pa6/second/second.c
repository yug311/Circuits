#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Gate
{
    char *type;
    int numInputs;
    char **inputs;
    int numOutputs;
    char **outputs;
    int numSelectors;
    char **selectors;
    int hidden;
    struct Gate *next;
} Gate;

typedef struct Data
{
    char *name;
    int value;
    struct Data *next;
} Data;

Data *datahead = NULL;
Gate *gates = NULL;
char ***hiddenGateStates = NULL;
int numHiddenGates = 0;
int *hiddenGateCols = NULL;
char **hiddenGateOutputs = NULL;
int numInput = 0;
int numOutput = 0; //number of output variables
char **inputvar = NULL;
char **outputvar = NULL;
int numOutputValues = 0; //2^numInput
int **outputValues = NULL;
int valid = 0;
int totaldone = 0;
int *gray_code = NULL;

void reflective_gray_code(int n, int *x)
{
    int size = 1 << n;
    for (int i = 0; i < size; i++)
    {
        int reflected_gray_code = i ^ (i >> 1);
        x[i] = reflected_gray_code;
    }
}

void print_cartesian_product(int rows, int* cols, char ***array, int current_row, int* current_combination) {
    // If we have reached the last row, print the combination
    if (current_row == rows) {
        // Print the elements of the combination
        hiddenGateOutputs = malloc(rows * sizeof(char*));
        for (int i = 0; i < rows; i++)
        {
            hiddenGateOutputs[i] = malloc(13 * sizeof(char));
            strcpy(hiddenGateOutputs[i], array[i][current_combination[i]]);      
        }

        //assign values to hidden gate outputs
        Gate *temp = gates;
        int num = 0;
        while(temp != NULL)
        {
            if(temp->type[0] == 'G')
            {
                strcpy(temp->type, hiddenGateOutputs[num]);
                num++;
            }
            temp = temp->next;
        }

        //assign input values to gates
        valid = 0;
        for(int x = 0; x<numOutputValues; x++)
        {
            Data *temp1 = datahead;
            temp1 = temp1->next->next;

            //extract the first n bits of x
            for(int y = numInput - 1; y>=0; y--)
            {
                temp1->value = (gray_code[x] >> y) & 1;
                temp1 = temp1->next;
            }

            Gate *gatepointer = gates;
            while(gatepointer != NULL)
            {
                if(strcmp(gatepointer->type, "AND") == 0 || strcmp(gatepointer->type, "OR") == 0 || strcmp(gatepointer->type, "XOR") == 0 || strcmp(gatepointer->type, "NOT") == 0)
                {
                    int input1 = 0;
                    int input2 = 0;
                    char *input1name = malloc(13 * sizeof(char));
                    strcpy(input1name, gatepointer->inputs[0]);
                    char *input2name;
                    if(gatepointer->numInputs == 2)
                    {
                        input2name = malloc(13 * sizeof(char));
                        strcpy(input2name, gatepointer->inputs[1]);
                    }

                    Data *temp2 = datahead;
                    while(temp2 != NULL)
                    {
                        if(strcmp(temp2->name, input1name) == 0)
                        {
                            input1 = temp2->value;
                        }

                        if(gatepointer->numInputs == 2)
                        {
                            if(strcmp(temp2->name, input2name) == 0)
                            {
                                input2 = temp2->value;
                            }
                        }

                        temp2 = temp2->next;
                    }

                    free(input1name);
                    if(gatepointer->numInputs == 2)
                        free(input2name);

                    Data *newData = malloc(sizeof(Data));
                    if(strcmp(gatepointer->type, "AND") == 0)
                    {
                        newData->value = input1 & input2;
                    }

                    else if(strcmp(gatepointer->type, "OR") == 0)
                    {
                        newData->value = input1 | input2;
                    }

                    else if(strcmp(gatepointer->type, "XOR") == 0)
                    {
                        newData->value = input1 ^ input2;
                    }

                    else if(strcmp(gatepointer->type, "NOT") == 0)
                    {
                        newData->value = !input1;
                    }
                    newData->name = malloc(13 * sizeof(char));
                    strcpy(newData->name, gatepointer->outputs[0]);
                    newData->next = NULL;

                    temp2 = datahead;
                    while(temp2->next != NULL) //insert last
                    {
                        temp2 = temp2->next;
                    }
                    temp2->next = newData;
                }

                else if(strcmp(gatepointer->type, "MULTIPLEXER") == 0)
                {
                    int numSelectors = gatepointer->numSelectors;
                    int selectors[numSelectors];
                    for(int i = 0; i < numSelectors; i++)
                    {
                        char *selectorname = malloc(13 * sizeof(char));
                        strcpy(selectorname, gatepointer->selectors[i]);
                        Data *temp2 = datahead;
                        while(temp2 != NULL)
                        {
                            if(strcmp(temp2->name, selectorname) == 0)
                            {
                                selectors[i] = temp2->value;
                            }
                            temp2 = temp2->next;
                        }
                    }

                    int selector = 0;
                    for(int i = 0; i < numSelectors; i++)
                    {
                        selector = selector << 1;
                        selector = selector | selectors[i];
                    }

                    int *grays = malloc(gatepointer->numInputs * sizeof(int));
                    reflective_gray_code(gatepointer->numSelectors, grays);
                    int m = 0;
                    for(int z = 0; z < gatepointer->numInputs; z++)
                    {
                        if(grays[z] == selector)
                        {
                            m = z;
                            break;
                        }
                    }
                    free(grays);

                    int input = 0;
                    Data *newData = malloc(sizeof(Data));
                    newData->name = malloc(13 * sizeof(char));
                    strcpy(newData->name, gatepointer->outputs[0]);
                    newData->next = NULL;

                    Data *temp2 = datahead;
                    while(temp2 != NULL)
                    {
                        if(strcmp(temp2->name, gatepointer->inputs[m]) == 0)
                        {
                            input = temp2->value;
                            break;
                        }
                        temp2 = temp2->next;
                    }
                    newData->value = input;

                    temp2 = datahead;
                    while(temp2->next != NULL) //insert last
                    {
                        temp2 = temp2->next;
                    }
                    temp2->next = newData;
                }

                else if(strcmp(gatepointer->type, "DECODER") == 0)
                {
                    int numSelectors = gatepointer->numInputs;
                    int selectors[numSelectors];
                    for(int i = 0; i < numSelectors; i++)
                    {
                        char *selectorname = malloc(13 * sizeof(char));
                        strcpy(selectorname, gatepointer->inputs[i]);
                        Data *temp2 = datahead;
                        while(temp2 != NULL)
                        {
                            if(strcmp(temp2->name, selectorname) == 0)
                            {
                                selectors[i] = temp2->value;
                            }
                            temp2 = temp2->next;
                        }
                    }

                    int selector = 0;
                    for(int i = 0; i < numSelectors; i++)
                    {
                        selector = selector << 1;
                        selector = selector | selectors[i];
                    }

                    int *grays = malloc(gatepointer->numOutputs * sizeof(int));
                    reflective_gray_code(gatepointer->numInputs, grays);
                    int m = 0;
                    for(int z = 0; z < gatepointer->numOutputs; z++)
                    {
                        if(grays[z] == selector)
                        {
                            m = z;
                            break;
                        }
                    }
                    free(grays);

                    //insert values into datahead as 0 but change the value of the selector to 1
                    for(int i = 0; i < gatepointer->numOutputs; i++)
                    {
                        Data *newData = malloc(sizeof(Data));
                        newData->name = malloc(13 * sizeof(char));
                        strcpy(newData->name, gatepointer->outputs[i]);
                        newData->next = NULL;
                        if(i == m)
                            newData->value = 1;
                        else
                            newData->value = 0;

                        Data *temp2 = datahead;
                        while(temp2->next != NULL) //insert last
                        {
                            temp2 = temp2->next;
                        }
                        temp2->next = newData;
                    }

                }
                gatepointer = gatepointer->next;
            }

        //CHECK datavalues and compare to output values
            for(int i = 0; i < numOutput; i++)
            {
                int output = 0;
                Data *temp2 = datahead;
                while(temp2 != NULL)
                {
                    if(strcmp(temp2->name, outputvar[i]) == 0)
                    {
                        output = temp2->value;
                    }
                    temp2 = temp2->next;
                }

                if(output != outputValues[i][x])
                {
                    valid = 1;
                    break;
                }
            }

            //reset datahead by removing all the values except the first 2 + numInput values
            Data *temp2 = datahead;
            temp2 = temp2->next->next;
            for(int i = 0; i < numInput - 1; i++)
            {
                temp2 = temp2->next;
            }
            // free remaining values
            Data *temp3 = temp2->next;
            while(temp3 != NULL)
            {
                Data *temp4 = temp3;
                temp3 = temp3->next;
                free(temp4->name);
                free(temp4);
            }
            temp2->next = NULL;

            if(valid == 1)
            {
                break;
            }
        }

        if(valid == 0 && totaldone == 0)
        {
            totaldone = 1;
            for(int i = 0; i < rows; i++)
            {
                printf("G%d %s\n", i + 1, hiddenGateOutputs[i]);
            }
        }

        if(totaldone == 0)
        {
            Gate *temp4 = gates;
            while(temp4 != NULL)
            {
                if(temp4->hidden == 1)
                {
                    strcpy(temp4->type, "G");
                }
                temp4 = temp4->next;
            }
        }

        for(int i = 0; i < rows; i++)
        {
            free(hiddenGateOutputs[i]);
        }
        free(hiddenGateOutputs);
        return;
    }

    // Recursively generate combinations for the next row
    for (int i = 0; i < cols[current_row]; i++)
    {
        current_combination[current_row] = i;
        print_cartesian_product(rows, cols, array, current_row + 1, current_combination);
    }
}

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "r");

    char *type = malloc(13 * sizeof(char));
    fscanf(fp, "%s", type);

    if(strcmp(type, "OUTPUTVAR") == 0)
    {
        fscanf(fp, "%d", &numOutput);

        outputvar = malloc(numOutput * sizeof(char*));
        for(int i = 0; i < numOutput; i++)
        {
            outputvar[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s", outputvar[i]);
        }
    }

    else if(strcmp(type, "INPUTVAR") == 0)
    {
        fscanf(fp, "%d", &numInput);

        inputvar = malloc(numInput * sizeof(char*));
        for(int i = 0; i < numInput; i++)
        {
            inputvar[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s", inputvar[i]);
        }
    }

    fscanf(fp, "%s", type);

    if(strcmp(type, "OUTPUTVAR") == 0)
    {
        fscanf(fp, "%d", &numOutput);

        outputvar = malloc(numOutput * sizeof(char*));
        for(int i = 0; i < numOutput; i++)
        {
            outputvar[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s", outputvar[i]);
        }
    }

    else if(strcmp(type, "INPUTVAR") == 0)
    {
        fscanf(fp, "%d", &numInput);

        inputvar = malloc(numInput * sizeof(char*));
        for(int i = 0; i < numInput; i++)
        {
            inputvar[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s", inputvar[i]);
        }
    }

    numOutputValues = 1 << numInput;
    fscanf(fp, "%s\n", type);
    outputValues = malloc(numOutput * sizeof(int*));
    for(int i = 0; i < numOutput; i++)
    {
        fscanf(fp, "%s\n", type);
        outputValues[i] = malloc(numOutputValues * sizeof(int));
        for(int j = 0; j < numOutputValues; j++)
        {
            fscanf(fp, "%d ", &outputValues[i][j]);
        }
    }

    gray_code = malloc(numOutputValues * sizeof(int));
    reflective_gray_code(numInput, gray_code);

    gates = NULL;
    while(!feof(fp))
    {
        char *gatename = malloc(13 * sizeof(char));
        int numGateInputs = 0;
        int numGateOutputs = 0; 
        int numSelectors = 0;
        int multihidden = 0;
        fscanf(fp, "%s\n", gatename);
        if(strcmp(gatename, "XOR") == 0 || strcmp(gatename, "AND") == 0 || strcmp(gatename, "OR") == 0)
        {
            numGateInputs = 2;
            numGateOutputs = 1;
        }

        else if(strcmp(gatename, "NOT") == 0)
        {
            numGateInputs = 1;
            numGateOutputs = 1;
        }

        else if(strcmp(gatename, "DECODER") == 0)
        {
            fscanf(fp, "%d\n", &numGateInputs);
            numGateOutputs = 1 << numGateInputs;
        }

        else if(strcmp(gatename, "MULTIPLEXER") == 0)
        {
            fscanf(fp, "%d\n", &numGateInputs);
            numGateOutputs = 1;
            int temp = numGateInputs;
            while(temp > 1)
            {
                temp = temp >> 1;
                numSelectors++;
            }
        }

        else // G/Hidden Gate
        {
            int totalargs = 0;
            fscanf(fp, "%d\n", &totalargs);
            hiddenGateStates = realloc(hiddenGateStates, (numHiddenGates + 1) * sizeof(char**));

            if(totalargs == 3)
            {
                numGateInputs = 2;
                numGateOutputs = 1;
                hiddenGateStates[numHiddenGates] = malloc(3 * sizeof(char*));
                hiddenGateStates[numHiddenGates][0] = malloc(13 * sizeof(char));
                hiddenGateStates[numHiddenGates][1] = malloc(13 * sizeof(char));
                hiddenGateStates[numHiddenGates][2] = malloc(13 * sizeof(char));
                strcpy(hiddenGateStates[numHiddenGates][0], "OR");
                strcpy(hiddenGateStates[numHiddenGates][1], "AND");
                strcpy(hiddenGateStates[numHiddenGates][2], "XOR");

                hiddenGateCols = realloc(hiddenGateCols, (numHiddenGates + 1) * sizeof(int));
                hiddenGateCols[numHiddenGates] = 3;

            }

            else if(totalargs == 2)
            {
                numGateInputs = 1;
                numGateOutputs = 1;
                hiddenGateStates[numHiddenGates] = malloc(1 * sizeof(char*));
                hiddenGateStates[numHiddenGates][0] = malloc(13 * sizeof(char));
                strcpy(hiddenGateStates[numHiddenGates][0], "NOT");

                hiddenGateCols = realloc(hiddenGateCols, (numHiddenGates + 1) * sizeof(int));
                hiddenGateCols[numHiddenGates] = 1;

            }

            else if(totalargs > 3)
            {
                if(totalargs % 2 == 0) //this means it is a decoder
                {
                    hiddenGateStates[numHiddenGates] = malloc(1 * sizeof(char*));
                    hiddenGateStates[numHiddenGates][0] = malloc(13 * sizeof(char));
                    strcpy(hiddenGateStates[numHiddenGates][0], "DECODER");

                    hiddenGateCols = realloc(hiddenGateCols, (numHiddenGates + 1) * sizeof(int));
                    hiddenGateCols[numHiddenGates] = 1;

                    //total args = n + 2^n where n is the number of inputs
                    for(int i = 1; i < 4; i++)
                    {
                        if(totalargs == i + (1 << i))
                        {
                            numGateInputs = i;
                            numGateOutputs = 1 << i;
                            break;
                        }
                    }
                }

                else //this means it is a multiplexer
                {
                    strcpy(gatename, "MULTIPLEXER");
                    multihidden = 1;
                    hiddenGateStates[numHiddenGates] = malloc(1 * sizeof(char*));
                    hiddenGateStates[numHiddenGates][0] = malloc(13 * sizeof(char));
                    strcpy(hiddenGateStates[numHiddenGates][0], "MULTIPLEXER");

                    hiddenGateCols = realloc(hiddenGateCols, (numHiddenGates + 1) * sizeof(int));
                    hiddenGateCols[numHiddenGates] = 1;

                    for(int i = 1; i < 4; i++)
                    {
                        if(totalargs - 1 == i + (1 << i))
                        {
                            numGateInputs = 1 << i;
                            numGateOutputs = 1;
                            break;
                        }
                    }

                    int temp = numGateInputs;
                    while(temp > 1)
                    {
                        temp = temp >> 1;
                        numSelectors++;
                    }
                }

            }
        numHiddenGates++;

        }

        Gate *newGate = malloc(sizeof(Gate));
        newGate->type = malloc(13 * sizeof(char));
        strcpy(newGate->type, gatename);
        newGate->numInputs = numGateInputs;
        newGate->numOutputs = numGateOutputs;
        newGate->inputs = malloc(numGateInputs * sizeof(char*));
        newGate->outputs = malloc(numGateOutputs * sizeof(char*));
        newGate->next = NULL;
        newGate->selectors = NULL;
        newGate->numSelectors = 0;

        for(int i = 0; i < numGateInputs; i++)
        {
            newGate->inputs[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s ", newGate->inputs[i]);
        }

        if(strcmp(newGate->type, "MULTIPLEXER") == 0)
        {
            if(multihidden == 1)
            {
                strcpy(newGate->type, "G10");
            }
            newGate->selectors = malloc(numSelectors * sizeof(char*));
            for(int i = 0; i < numSelectors; i++)
            {
                newGate->selectors[i] = malloc(13 * sizeof(char));
                fscanf(fp, "%s ", newGate->selectors[i]);
            }
            newGate->numSelectors = numSelectors;
        }

        for(int i = 0; i < numGateOutputs; i++)
        {
            newGate->outputs[i] = malloc(13 * sizeof(char));
            fscanf(fp, "%s ", newGate->outputs[i]);
        }

        if(newGate->type[0] == 'G')
        {
            newGate->hidden = 1;
        }

        else
        {
            newGate->hidden = 0;
        }

        //add to linked list at end
        if(gates == NULL)
        {
            gates = newGate;
        }

        else
        {
            Gate *temp = gates;
            while(temp->next != NULL)
            {
                temp = temp->next;
            }
            temp->next = newGate;
        }
        free(gatename);
    }

    datahead = malloc(sizeof(Data));
    datahead->name = malloc(13 * sizeof(char));
    strcpy(datahead->name, "1");
    datahead->value = 1;

    Data *temp = malloc(sizeof(Data));
    temp->name = malloc(13 * sizeof(char));
    strcpy(temp->name, "0");
    temp->value = 0;
    temp->next = NULL;
    datahead->next = temp;

    for(int i = 0; i < numInput; i++)
    {
        Data *newData = malloc(sizeof(Data));
        newData->name = malloc(13 * sizeof(char));
        strcpy(newData->name, inputvar[i]);
        newData->value = 0;
        newData->next = NULL;

        temp->next = newData;
        temp = newData;
    }

    int current_combination[numHiddenGates];
    
    // Start recursion from the first row
    print_cartesian_product(numHiddenGates, hiddenGateCols, hiddenGateStates, 0, current_combination);
    if(totaldone == 0)
        printf("INVALID\n");

    free(type);
    for(int i = 0; i < numOutput; i++)
    {
        free(outputvar[i]);
    }
    free(outputvar);

    for(int i = 0; i < numInput; i++)
    {
        free(inputvar[i]);
    }
    free(inputvar);

    for(int i = 0; i < numOutput; i++)
    {
        free(outputValues[i]);
    }
    free(outputValues);

    //free datahead
    Data *temp1 = datahead;
    while(temp1 != NULL)
    {
        Data *temp2 = temp1;
        temp1 = temp1->next;
        free(temp2->name);
        free(temp2);
    }

    // free gates
    Gate *temp3 = gates;
    while(temp3 != NULL)
    {
        Gate *temp4 = temp3;
        temp3 = temp3->next;
        free(temp4->type);
        for(int i = 0; i < temp4->numInputs; i++)
        {
            free(temp4->inputs[i]);
        }
        free(temp4->inputs);
        for(int i = 0; i < temp4->numOutputs; i++)
        {
            free(temp4->outputs[i]);
        }
        free(temp4->outputs);
        for(int i = 0; i < temp4->numSelectors; i++)
        {
            free(temp4->selectors[i]);
        }
        free(temp4->selectors);
        free(temp4);
    }

    //free hiddenGateStates
    for(int i = 0; i < numHiddenGates; i++)
    {
        for(int j = 0; j < hiddenGateCols[i]; j++)
        {
            free(hiddenGateStates[i][j]);
        }
        free(hiddenGateStates[i]);
    }
    free(hiddenGateStates);

    free(hiddenGateCols);

    free(gray_code);

    fclose(fp);
}