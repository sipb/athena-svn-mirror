<?xml version='1.0'?>
<xsl:stylesheet  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

<!-- copyright (C) 2001 Sun Microsystems, Inc.  -->

<xsl:template match="/">
	<toc>
	<xsl:text>&#10;</xsl:text>
        <xsl:apply-templates select="book[@id]|part|article[@id]|reference"> 
		<xsl:with-param name="toclevel" select="0"/>
	</xsl:apply-templates>
	</toc>
</xsl:template>

<xsl:template match="book">
	<xsl:param name="toclevel"/>
        <xsl:apply-templates select="article[@id]|chapter[@id]|appendix[@id]|part[@id]">
		<xsl:with-param name="toclevel" select="$toclevel+1"/>
	</xsl:apply-templates>
</xsl:template>

<xsl:template match="reference">
	<xsl:param name="toclevel"/>
	<xsl:apply-templates select="refentry[@id]">
		<xsl:with-param name="toclevel" select="$toclevel+1"/>
	</xsl:apply-templates>
</xsl:template>

<!--	match part in two different ways - making "id-less" parts skip a level,
	mimicking it all having been an article. 
-->

<xsl:template match="part">
	<xsl:param name="toclevel"/>
	<xsl:apply-templates select="chapter[@id]/sect1[@id]">
		<xsl:with-param name="toclevel" select="$toclevel+1"/>
	</xsl:apply-templates>
</xsl:template>

<xsl:template match="part[@id]">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
			<xsl:value-of select="@id"/>
		</xsl:attribute>
		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
		<xsl:apply-templates select="appendix[@id]|article[@id]|chapter[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
		</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="article">
	<xsl:param name="toclevel"/>
     	<xsl:apply-templates select="sect1[@id]|appendix[@id]">
		<xsl:with-param name="toclevel" select="$toclevel+1"/>
	</xsl:apply-templates>
</xsl:template>

<xsl:template match="appendix">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
        	<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
        	<xsl:apply-templates select="sect1[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
  </xsl:template>

<xsl:template match="chapter">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
        	<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
        	<xsl:apply-templates select="sect1[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="sect1">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
		<xsl:apply-templates select="sect2[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="sect2">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
       		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
       		<xsl:apply-templates select="sect3[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="sect3">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
       		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
       		<xsl:apply-templates select="sect4[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="sect4">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
       		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
		<xsl:apply-templates select="sect5[@id]">
			<xsl:with-param name="toclevel" select="$toclevel+1"/>
        	</xsl:apply-templates>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>

<xsl:template match="sect5">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
       		<xsl:value-of select="title"/>
		<xsl:text>&#10;</xsl:text>
	</xsl:element>
	<xsl:text>&#10;</xsl:text>
</xsl:template>
  
<xsl:template match="refentry">
	<xsl:param name="toclevel"/>
	<xsl:element name="tocsect{$toclevel}">
		<xsl:attribute name="linkid">
                        <xsl:value-of select="@id"/>
                </xsl:attribute>
       		<xsl:value-of select="refmeta/refentrytitle"/>
       		<!-- <xsl:value-of select="@id"/> -->
		<xsl:text>&#10;</xsl:text>
	</xsl:element>
        <xsl:text>&#10;</xsl:text>
</xsl:template>
  
</xsl:stylesheet>
