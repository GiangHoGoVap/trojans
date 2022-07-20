import { Module } from '@nestjs/common';
import { MongooseModule } from '@nestjs/mongoose';
import { GraphQLModule } from '@nestjs/graphql';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { UserModule } from './user/user.module';

@Module({
  imports: [
    MongooseModule.forRoot('mongodb://local-host/chatbot-widget-demo'),
    GraphQLModule.forRoot({
      autoSchemaFile: 'schema.gql'
    }),
    UserModule],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
