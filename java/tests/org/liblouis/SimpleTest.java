package org.liblouis;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.List;
import java.util.ArrayList;

import org.testng.annotations.*;
import org.testng.Assert;

import org.liblouis.LibLouisUTDML;
import org.liblouis.LogLevel;

public class SimpleTest {
  LibLouisUTDML lbu;
  @BeforeClass
  public void LoadLibLouisUTDML() throws Exception {
    LibLouisUTDML.loadLibrary(System.getProperty("liblouis.dir"), System.getProperty("liblouis.ext"));
    lbu = LibLouisUTDML.getInstance();
    lbu.setDataPath(new File("testdata").getAbsolutePath());
    lbu.setLogLevel(LogLevel.FATAL);
  }
  @DataProvider(name="translateFileTest")
  public Object[][] translateFileData() {
    List<Object> testCaseData = new ArrayList();
    testCaseData.add("nimas.cfg");
    testCaseData.add(new File("testdata", "list-test.xml").getAbsolutePath());
    testCaseData.add(new File("testdata", "output.utd").getAbsolutePath());
    testCaseData.add(null);
    testCaseData.add(null);
    testCaseData.add(0);
    return new Object[][] {testCaseData.toArray()};
  }
  @Test(dataProvider="translateFileTest")
  public void testTranslateFile(String configList, String inFile, String expectedFile, String logFile, String settings, int mode) throws Exception {
    // Perform the translation of the file
    if (!lbu.translateFile(configList, inFile, new File("testdata", "actual.utd").getAbsolutePath(), logFile, settings, mode)) {
      throw new Exception("LibLouisUTDML could not perform translation of the file " + inFile);
    }
    // Now compare the results
    BufferedReader actualReader = new BufferedReader(new FileReader(new File("testdata", "actual.utd")));
    BufferedReader expectedReader = new BufferedReader(new FileReader(new File(expectedFile)));
    String actualLine;
    String expectedLine;
    // Do not use the && shortcut as we always want to read both files
    while (((actualLine = actualReader.readLine()) != null) & ((expectedLine = expectedReader.readLine()) != null)) {
      if (!actualLine.equals(expectedLine)) {
        // Set one of the line strings to null so that fail will be detected
        actualLine = null;
        break;
      }
    }
    // Check for failure, only one of actualLine or expectedLine will be null
    if ((actualLine == null) ^ (expectedLine == null)) {
      Assert.fail("The actual file does not match the expected file");
    }
  }
}
