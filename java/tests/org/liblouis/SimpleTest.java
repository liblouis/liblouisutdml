package org.liblouis;

import java.io.IOException;
import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.List;
import java.util.ArrayList;

import javax.xml.parsers.*;
import javax.xml.xpath.*;

import org.w3c.dom.*;
import org.xml.sax.SAXException;

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
  private String getTextFromNode(XPathExpression expr, Node searchNode) throws XPathExpressionException {
    NodeList resultList = (NodeList)expr.evaluate(searchNode, XPathConstants.NODESET);
    if (resultList.getLength() == 0) {
      return null;
    }
    return resultList.item(0).getNodeValue();
  }
  @DataProvider(name="translateFileTest")
  public Object[][] translateFileData() throws ParserConfigurationException, SAXException, XPathExpressionException, IOException {
    DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance();
    domFactory.setNamespaceAware(true);
    DocumentBuilder builder = domFactory.newDocumentBuilder();
    Document doc = builder.parse(new File("testdata", "translateFileTests.xml").getAbsolutePath());
    XPathFactory factory = XPathFactory.newInstance();
    XPath xpath = factory.newXPath();
    XPathExpression testsExpr = xpath.compile("/tests/translateFileTest");
    XPathExpression configListExpr = xpath.compile("configList/text()");
    XPathExpression inFileExpr = xpath.compile("inFile/text()");
    XPathExpression expectedOutFileExpr = xpath.compile("expectedOutFile/text()");
    XPathExpression modeExpr = xpath.compile("mode/text()");
    XPathExpression logFileExpr = xpath.compile("logFile/text()");
    XPathExpression settingsExpr = xpath.compile("settings/text()");
    NodeList testNodes = (NodeList)testsExpr.evaluate(doc, XPathConstants.NODESET);
    List<Object[]> tests = new ArrayList<Object[]>();
    Object[] testCase;
    String modeStr;
    for (int i = 0; i < testNodes.getLength(); i++) {
      testCase = new Object[6];
      if ((testCase[0] = getTextFromNode(configListExpr, testNodes.item(i))) == null) {
        System.out.println("Test with no configList being skipped");
        continue;
      }
      if ((testCase[1] = getTextFromNode(inFileExpr, testNodes.item(i))) == null) {
        System.out.println("Test has no inFile so skipping");
        continue;
      }
      if ((testCase[2] = getTextFromNode(expectedOutFileExpr, testNodes.item(i))) == null) {
        System.out.println("Test has no expectedOutFile so skipping");
        continue;
      }
      testCase[3] = getTextFromNode(logFileExpr, testNodes.item(i));
      testCase[4] = getTextFromNode(settingsExpr, testNodes.item(i));
      testCase[5] = 0;
      try {
        if ((modeStr = getTextFromNode(modeExpr, testNodes.item(i))) != null) {
          testCase[5] = Integer.parseInt(modeStr);
        }
      } catch(Exception e) {
        testCase[5] = 0;
      }
      tests.add(testCase);
    }
    return tests.toArray(new Object[tests.size()][6]);
  }
  @Test(dataProvider="translateFileTest")
  public void testTranslateFile(String configList, String inFile, String expectedFile, String logFile, String settings, int mode) throws Exception {
    String absoluteLogFilePath = null;
    if (logFile != null)
    {
      absoluteLogFilePath = new File(logFile).getAbsolutePath();
    }
    // Perform the translation of the file
    if (!lbu.translateFile(configList, new File(inFile).getAbsolutePath(), new File("testdata", "actual.utd").getAbsolutePath(), absoluteLogFilePath, settings, mode)) {
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
