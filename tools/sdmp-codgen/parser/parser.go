package parser

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os"
	"strings"
)

//********************************************************************************************************************
func CheckEqualType(StructDataType string, DataTypes []string) bool {
	for _, DataType := range DataTypes {
		if strings.Contains(StructDataType, DataType) {
			return true
		}
	}
	return false
}

//********************************************************************************************************************
func GetStructrures(InputFiles []string) (AllStructs map[string]map[string]string, errret error) {

	var RLine string
	var StructType string
	var StructData map[string]string

	AllStructs = make(map[string]map[string]string)

	// While on file list
	fmt.Print("### PARSING FILES \n")
	for _, InputFileName := range InputFiles {

		// Open file and reader
		fmt.Printf("###=== Opening input file [%s]\n", InputFileName)
		InFile, err := os.Open(InputFileName);
		if err != nil {
			errret = errors.New(fmt.Sprintf("[GetStructrures]: Error opening File [%s]", InputFileName))
			return AllStructs, errret
		}
		FileReader := bufio.NewReader(InFile)
		if FileReader == nil {
			errret = errors.New(fmt.Sprintf("[GetStructrures]: Error opening Reader for File [%s]", InputFileName))
			return AllStructs, errret
		}

		//Parsing file
		fmt.Printf("###===--- Parsing file \n")
		for {
			RLine, err = FileReader.ReadString('\n')
			if err == io.EOF {
				break
			}
			// Search structure contains ("typedef", "__packed__")
			if strings.Contains(RLine, "typedef") && strings.Contains(RLine, "__packed__") {
				if !strings.Contains(RLine, "{") { // If "{" not fount on line read string to "{" symbol
					if _, err = FileReader.ReadString('{'); err == io.EOF {
						break
					}
				}

				// Clear temporary map
				StructType = ""
				StructData = make(map[string]string)

				// Read the body structure to "}"
				RLine, err = FileReader.ReadString('}') // reading body structure
				if err == io.EOF {
					break
				}

				// Parse struct data lines and fill struct data array
				BodyReader := bufio.NewScanner(strings.NewReader(strings.TrimRight(strings.TrimLeft(RLine, " "), "}")))
				for BodyReader.Scan() {
					FieldsStructLine := strings.SplitN(strings.TrimLeft(strings.TrimRight(BodyReader.Text(), ";"), " "), " ", 2)
					if len(FieldsStructLine) > 1 {
						fmt.Printf("###===------TYPE: [%s] NAME:[%s] \n", FieldsStructLine[0], FieldsStructLine[1])
						StructData[FieldsStructLine[1]] = FieldsStructLine[0]
					}
				}

				fmt.Println("=== STR NAME ===")
				fmt.Println(StructData)
				fmt.Println("=== END STR NAME ===")

				// Reading Struct Name
				RLine, err = FileReader.ReadString(';') // reading struct name
				if err == io.EOF {
					break
				}

				//Append data to final structure
				StructType = strings.TrimRight(strings.TrimLeft(RLine, " "), ";")
				if len(StructType) > 0 {
					AllStructs[StructType] = StructData
					fmt.Printf("###===--- FOUND [%s]\n", StructType)
				}
			}
		}
	} // End of while on file list

	// If Structs not found (reset final data)
	if len(AllStructs) < 1 {
		AllStructs = nil
		errret = errors.New(fmt.Sprintf("Structs not found"))
		return AllStructs, errret
	}

	return AllStructs, errret
}
