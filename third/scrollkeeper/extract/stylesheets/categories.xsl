<?xml version='1.0'?>
<!DOCTYPE xsl:stylesheet [
<!ENTITY sep '" "'>
<!ENTITY comma '", "'>
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>
<xsl:output method="html" indent="yes"/>

<xsl:template match="ScrollKeeperContentsList">
  <html>
    <head><title>ScrollKeeper Contents List Categories</title></head>
      <body bgcolor="#FFFFFF">
        <h1 align="center">ScrollKeeper Contents List Categories</h1>
        <xsl:apply-templates select="sect">
            <xsl:with-param name="indentlevel" select="0"/>
        </xsl:apply-templates>
      </body>
  </html>
</xsl:template>

<xsl:template match="sect">
    <xsl:param name="indentlevel"/>
    <blockquote>
    <xsl:if test="$indentlevel='0'">
        <b><xsl:value-of select="title"/></b>
    </xsl:if>
    <xsl:if test="$indentlevel>'0'">
        <xsl:value-of select="title"/>
    </xsl:if>
    <xsl:apply-templates select="sect">
        <xsl:with-param name="indentlevel" select="$indentlevel+1"/>
    </xsl:apply-templates>
    </blockquote>
</xsl:template>

</xsl:stylesheet>

