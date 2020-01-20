// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
/**
 * \class document::Document
 * \ingroup fieldvalue
 *
 * \brief A class for storing structured data.
 *
 * The document is typically the field value handled by clients. It is documents
 * that are sent around within Vespa. The other fieldvalues are used within
 * documents, and documents are field values themselves, as we can store
 * documents within documents, and also to ensure the user interface for
 * documents is equal to that of other structured types.
 *
 * @see documentmanager.h
 */
#pragma once

#include "structfieldvalue.h"
#include <vespa/document/base/documentid.h>
#include <vespa/document/base/field.h>

namespace document {

class Document : public StructuredFieldValue
{
private:
    DocumentId _id;
    StructFieldValue _fields;

    // To avoid having to return another container object out of docblocks
    // the meta data has been added to document. This will not be serialized
    // with the document and really doesn't belong here!
    int64_t _lastModified;

public:
    typedef std::unique_ptr<Document> UP;
    typedef std::shared_ptr<Document> SP;

    static constexpr uint16_t getNewestSerializationVersion() { return 8; }
    static const DataType & verifyDocumentType(const DataType *type);
    static void verifyIdAndType(const DocumentId & id, const DataType *type);

    Document();
    Document(const Document&);
    Document(const DataType &, DocumentId id);
    Document(const DocumentTypeRepo& repo, vespalib::nbostream& stream);
    ~Document() override;

    void setRepo(const DocumentTypeRepo & repo);
    const DocumentTypeRepo * getRepo() const { return _fields.getRepo(); }

    Document& operator=(const Document&);

    void accept(FieldValueVisitor &visitor) override { visitor.visit(*this); }
    void accept(ConstFieldValueVisitor &visitor) const override { visitor.visit(*this); }

    void setType(const DataType & type) override;
    const DocumentType& getType() const;
    const DocumentId& getId() const { return _id; }
    DocumentId & getId() { return _id; }

    /**
     * Get the last modified timestamp of a document if this is a document
     * you have retrieved from a docblock.
     */
    int64_t getLastModified() const { return _lastModified; }

    /**
     * Set the last modified timestamp that will be retrieved with
     * getLastModifiedTime().
     */
    void setLastModified(const int64_t lastModified) { _lastModified = lastModified; }

    const StructFieldValue& getFields() const { return _fields; }
    StructFieldValue& getFields() { return _fields; }

    const Field& getField(vespalib::stringref name) const override { return _fields.getField(name); }
    bool hasField(vespalib::stringref name) const override { return _fields.hasField(name); }

    void clear() override;

    bool hasChanged() const override;

    // FieldValue implementation.
    FieldValue& assign(const FieldValue&) override;
    int compare(const FieldValue& other) const override;
    Document* clone() const override { return new Document(*this); }
    void printXml(XmlOutputStream& out) const override;
    void print(std::ostream& out, bool verbose, const std::string& indent) const override;

    // Specialized serialization functions, Only used for testing legacy stuff
    void serializeHeader(vespalib::nbostream& stream) const;
    void serializeBody(vespalib::nbostream& stream) const;

    void deserialize(const DocumentTypeRepo& repo, vespalib::nbostream & os);
    /** Deserialize document contained in given bytebuffers. */
    void deserialize(const DocumentTypeRepo& repo, vespalib::nbostream & body, vespalib::nbostream & header);

    /** Undo fieldvalue's toXml override for document. */
    std::string toXml() const { return toXml(""); }
    std::string toXml(const std::string& indent) const override;

    bool empty() const override { return _fields.empty(); }

    uint32_t calculateChecksum() const;

    DECLARE_IDENTIFIABLE_ABSTRACT(Document);

    void setFieldValue(const Field& field, FieldValue::UP data) override;
private:
    void deserializeHeader(const DocumentTypeRepo& repo, vespalib::nbostream & header);
    void deserializeBody(const DocumentTypeRepo& repo, vespalib::nbostream & body);
    bool hasBodyField() const;
    bool hasFieldValue(const Field& field) const override { return _fields.hasValue(field); }
    void removeFieldValue(const Field& field) override { _fields.remove(field); }
    FieldValue::UP getFieldValue(const Field& field) const override { return _fields.getValue(field); }
    bool getFieldValue(const Field& field, FieldValue& value) const override { return _fields.getValue(field, value); }

    StructuredIterator::UP getIterator(const Field* first) const override;
};

}  // document
